// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "global-tstate.h"
#include "envmap.h"

#include "scene.h"


using namespace snogray;
using namespace std;

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (surface_iterator_t oi = surfaces.begin(); oi != surfaces.end(); oi++)
    delete *oi;
  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    delete *li;
  for (material_iterator_t mi = materials.begin(); mi != materials.end(); mi++)
    delete *mi;

  delete env_map;

  if (light_map != env_map)
    delete light_map;
}


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct SceneClosestIntersectCallback : Space::IntersectCallback
{
  SceneClosestIntersectCallback (Ray &_ray, IsecParams &_isec_params,
				 Trace &_trace)
    : IntersectCallback (&_trace.global.stats.space_intersect),
      ray (_ray), isec_params (_isec_params),
      closest (0), trace (_trace), num_calls (0)
  { }

  virtual void operator() (Surface *);

  Ray &ray;

  IsecParams &isec_params;

  // The the closest intersecting surface we've found
  //
  const Surface *closest;

  Trace &trace;

  unsigned num_calls;
};

void
SceneClosestIntersectCallback::operator () (Surface *surface)
{
  if (surface != trace.horizon_hint)
    {
      if (surface->intersect (ray, isec_params, trace.origin_count (surface)))
	closest = surface;

      num_calls++;
    }
}

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface *
Scene::intersect (Ray &ray, IsecParams &isec_params, Trace &trace)
  const
{
  trace.global.stats.scene_intersect_calls++;

  // Make a callback, and call it for each surface that may intersect
  // the ray.

  SceneClosestIntersectCallback closest_isec_cb (ray, isec_params, trace);

  // If there's a horizon hint, try to use it to reduce the horizon
  // before searching -- space searching can dramatically improve given
  // a limited search space.
  //
  const Surface *hint = trace.horizon_hint;
  if (hint)
    {
      if (hint->intersect (ray, isec_params, trace.origin_count (hint)))
	{
	  closest_isec_cb.closest = hint;
	  trace.global.stats.horizon_hint_hits++;
	}
      else
	trace.global.stats.horizon_hint_misses++;
    }

  space.for_each_possible_intersector (ray, closest_isec_cb);

  trace.global.stats.surface_intersect_calls += closest_isec_cb.num_calls;

  // Update the horizon hint to reflect what we found (0 if nothing).
  //
  trace.horizon_hint = closest_isec_cb.closest;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct SceneShadowCallback : Space::IntersectCallback
{
  SceneShadowCallback (const Ray &_light_ray, const Intersect &_isec,
		       Trace &_trace, const Light *_light)
    : IntersectCallback (&_trace.global.stats.space_shadow), 
      light_ray (_light_ray), isec (_isec),
      shadower (0), trace (_trace), light (_light),
      num_tests (0)
  { }

  virtual void operator() (Surface *);

  const Ray &light_ray;

  // Intersection which is possibly shadowed.
  //
  const Intersect &isec;

  // Shadowing surface discovered.
  //
  const Surface *shadower;

  Trace &trace;

  const Light *light;

  unsigned num_tests;
};

void
SceneShadowCallback::operator () (Surface *surface)
{
  Material::ShadowType shadow_type = surface->material->shadow_type;

  if (surface != isec.surface && shadow_type != Material::SHADOW_NONE)
    {
      num_tests++;

      if (surface->shadows (light_ray, isec))
	{
	  shadower = surface;

	  // Remember which surface we found, so we can try it first
	  // next time.
	  //
	  if (light)
	    trace.shadow_hints[light->num] = surface;

	  if (shadow_type == Material::SHADOW_OPAQUE)
	    //
	    // A simple opaque surface blocks everything; we can
	    // immediately return it; stop looking any further.
	    stop_iteration ();
	}
    }
}

// Return some surface shadowing LIGHT_RAY from LIGHT, or 0 if there is
// no shadowing surface.  If an surface it is returned, and it is _not_ an 
// "opaque" surface (shadow-type Material::SHADOW_OPAQUE), then it is
// guaranteed there are no opaque surfaces casting a shadow.
//
// ISEC is the intersection for which we are searching for shadow-casters.
//
// This is similar, but not identical to the behavior of the `intersect'
// method -- `intersect' always returns the closest surface and makes no
// guarantees about the properties of further intersections.
//
const Surface *
Scene::shadow_caster (const Ray &light_ray, const Intersect &isec,
		      Trace &trace, const Light *light)
  const
{
  trace.global.stats.scene_shadow_tests++;

  // See if this light has a shadow hint (the last surface that cast a shadow
  // from it); if it does, then try that surface first, as it stands a better
  // chance of hitting than usual (because nearby points are often obscured
  // from a given light by the same surface).
  //
  // Note that in the case where the hint refers to non-opaque surface,
  // we will return it immediately, just like an opaque surface.  This
  // will not cause errors, because the shadow-tracing "slow path"
  // (which will get used if a non-opaque surface is returned) still
  // does the right thing in this case, simply more slowly; in the case
  // where a new opaque surface is found, the hint will be updated
  // elsewhere (in Material::shadow actually).
  //
  if (light)
    {
      const Surface *hint = trace.shadow_hints[light->num];
      if (hint && hint != trace.origin)
	{
	  if (hint->shadows (light_ray, isec))
	    {
	      trace.global.stats.shadow_hint_hits++;
	      return hint;
	    }
	  else
	    // It didn't work; clear this hint out.
	    {
	      trace.global.stats.shadow_hint_misses++;
	      trace.shadow_hints[light->num] = 0;
	    }
	}
    }

  SceneShadowCallback shadow_cb (light_ray, isec, trace, light);

  space.for_each_possible_intersector (light_ray, shadow_cb);

  trace.global.stats.surface_intersects_tests += shadow_cb.num_tests;

  return shadow_cb.shadower;
}



// Returns the background color in the direction pointed to by RAY
//
Color
Scene::background (const Ray &ray) const
{
  if (env_map)
    {
      Color bg = env_map->map (ray.dir);

      // If there are separate environment and light-maps, it's likely that
      // the latter is HDR and the former LDR, so try to add highlight
      // information to the environment-map using the HDR light-map.
      //
      // This is done by detecting a "saturated" LDR background color --
      // within epsilon of 1.0 -- and using the corresponding light-map
      // color instead if it is greater than 1.0.  This is done
      // separately for each color component.
      //
      if (light_map && light_map != env_map)
	{
	  // "Saturation epsilon" -- if a color component is within this
	  // amount of 1.0, it is considered a possible "saturated" LDR
	  // pixel.
	  //
	  static const float SAT_EPS = 0.005;

	  // First we test the maximum component to see if it's possibly
	  // saturated; if not, there's no point in fetching the
	  // light-map color.
	  //
	  Color::component_t max = bg.max_component ();

	  if (max > 1-SAT_EPS && max < 1+SAT_EPS)
	    {
	      Color lmap_bg = light_map->map (ray.dir);

	      for (unsigned c = 0; c < Color::TUPLE_LEN; c++)
		if (bg[c] > 1-SAT_EPS && lmap_bg[c] > 1)
		  bg[c] = lmap_bg[c];
	    }
	}

      return bg;
    }
  else
    return bg_color;
}



void
Scene::set_background (const Color &col)
{
  if (env_map)
    {
      if (env_map != light_map)
	delete env_map;

      env_map = 0;
    }

  bg_color = col;
  bg_set = true;
}

void
Scene::set_background (const Envmap *map)
{
  if (env_map != light_map)
    delete env_map;

  env_map = map;
  bg_set = true;
}

void
Scene::set_light_map (const Envmap *lmap)
{
  if (light_map && light_map != env_map)
    delete light_map;

  light_map = lmap;
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
