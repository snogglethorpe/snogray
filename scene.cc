// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"

using namespace Snogray;
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
}


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct SceneClosestIntersectCallback : Voxtree::IntersectCallback
{
  SceneClosestIntersectCallback (Ray &_ray, IsecParams &_isec_params,
				 TraceState &_tstate, 
				 Voxtree::IsecStats *stats = 0)
    : IntersectCallback (stats), ray (_ray), isec_params (_isec_params),
      closest (0), tstate (_tstate), num_calls (0)
  { }

  virtual void operator() (Surface *);

  Ray &ray;

  IsecParams &isec_params;

  // The the closest intersecting surface we've found
  //
  const Surface *closest;

  TraceState &tstate;

  unsigned num_calls;
};

void
SceneClosestIntersectCallback::operator () (Surface *surface)
{
  if (surface != tstate.horizon_hint)
    {
      if (surface->intersect (ray, isec_params, tstate.origin_count (surface)))
	closest = surface;

      num_calls++;
    }
}

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface *
Scene::intersect (Ray &ray, IsecParams &isec_params, TraceState &tstate)
  const
{
  stats.scene_intersect_calls++;

  // Make a callback, and call it for each surface in the voxtree that may
  // intersect the ray.

  SceneClosestIntersectCallback
    closest_isec_cb (ray, isec_params, tstate, &stats.voxtree_intersect);

  // If there's a horizon hint, try to use it to reduce the horizon before
  // searching -- voxtree searching can dramatically improve given a
  // limited search space.
  //
  const Surface *hint = tstate.horizon_hint;
  if (hint)
    {
      if (hint->intersect (ray, isec_params, tstate.origin_count (hint)))
	{
	  closest_isec_cb.closest = hint;
	  stats.horizon_hint_hits++;
	}
      else
	stats.horizon_hint_misses++;
    }

  surface_voxtree.for_each_possible_intersector (ray, closest_isec_cb);

  stats.surface_intersect_calls += closest_isec_cb.num_calls;

  // Update the horizon hint to reflect what we found (0 if nothing).
  //
  tstate.horizon_hint = closest_isec_cb.closest;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct SceneShadowCallback : Voxtree::IntersectCallback
{
  SceneShadowCallback (const Light &_light, const Ray &_light_ray,
		       const Intersect &_isec,
		       TraceState &_tstate, Voxtree::IsecStats *stats = 0)
    : IntersectCallback (stats), 
      light (_light), light_ray (_light_ray), isec (_isec),
      shadower (0), tstate (_tstate), num_tests (0)
  { }

  virtual void operator() (Surface *);

  const Light &light;
  const Ray &light_ray;

  // Intersection which is possibly shadowed.
  //
  const Intersect &isec;

  // Shadowing surface discovered.
  //
  const Surface *shadower;

  TraceState &tstate;

  unsigned num_tests;
};

void
SceneShadowCallback::operator () (Surface *surface)
{
  Material::ShadowType shadow_type = surface->shadow_type;

  if (surface != tstate.origin && shadow_type != Material::SHADOW_NONE)
    {
      num_tests++;

      if (surface->shadows (light_ray, isec.smoothing_group, isec.back))
	{
	  shadower = surface;

	  // Remember which surface we found, so we can try it first
	  // next time.
	  //
	  tstate.shadow_hints[light.num] = surface;

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
Scene::shadow_caster (const Ray &light_ray, const Light &light,
		      const Intersect &isec, TraceState &tstate)
  const
{
  stats.scene_shadow_tests++;

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
  const Surface *hint = tstate.shadow_hints[light.num];
  if (hint && hint != tstate.origin)
    {
      if (hint->shadows (light_ray, isec.smoothing_group, isec.back))
	{
	  stats.shadow_hint_hits++;
	  return hint;
	}
      else
	// It didn't work; clear this hint out.
	{
	  stats.shadow_hint_misses++;
	  tstate.shadow_hints[light.num] = 0;
	}
    }

  SceneShadowCallback
    shadow_cb (light, light_ray, isec, tstate, &stats.voxtree_shadow);

  surface_voxtree.for_each_possible_intersector (light_ray, shadow_cb);

  stats.surface_intersects_tests += shadow_cb.num_tests;

  return shadow_cb.shadower;
}



// Iterate over every light, calculating its contribution the color of ISEC.
// LIGHT_MODEL is used to calculate the actual effect; COLOR is the "base color"
//
Color
Scene::illum (const Intersect &isec, const Color &color,
	      const LightModel &light_model, TraceState &tstate)
  const
{
  Color total_color;	// Accumulated colors from all light sources

  TraceState &sub_tstate
    = tstate.subtrace_state (TraceState::SHADOW, isec.surface);

  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    {
      const Light *light = *li;

      // If the dot-product of the impinging light-ray with the surface
      // normal is positive, that means the light is behind the surface,
      // so cannot light it ("self-shadowing"); otherwise, see if some
      // other surface casts a shadow.

      if (isec.normal.dot (light->pos - isec.point) >= -Eps)
	total_color += light->illum (isec, color, light_model, sub_tstate);
    }

  return total_color;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
