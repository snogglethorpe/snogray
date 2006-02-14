// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "cubetex.h"

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

  delete bg_cube;
}


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct SceneClosestIntersectCallback : Space::IntersectCallback
{
  SceneClosestIntersectCallback (Ray &_ray, IsecParams &_isec_params,
				 TraceState &_tstate, 
				 Space::IsecStats *stats = 0)
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

  // Make a callback, and call it for each surface that may intersect
  // the ray.

  SceneClosestIntersectCallback
    closest_isec_cb (ray, isec_params, tstate, &stats.space_intersect);

  // If there's a horizon hint, try to use it to reduce the horizon
  // before searching -- space searching can dramatically improve given
  // a limited search space.
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

  space.for_each_possible_intersector (ray, closest_isec_cb);

  stats.surface_intersect_calls += closest_isec_cb.num_calls;

  // Update the horizon hint to reflect what we found (0 if nothing).
  //
  tstate.horizon_hint = closest_isec_cb.closest;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct SceneShadowCallback : Space::IntersectCallback
{
  SceneShadowCallback (const Light &_light, const Ray &_light_ray,
		       const Intersect &_isec,
		       TraceState &_tstate, Space::IsecStats *stats = 0)
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

      if (surface->shadows (light_ray, isec))
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
      if (hint->shadows (light_ray, isec))
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
    shadow_cb (light, light_ray, isec, tstate, &stats.space_shadow);

  space.for_each_possible_intersector (light_ray, shadow_cb);

  stats.surface_intersects_tests += shadow_cb.num_tests;

  return shadow_cb.shadower;
}



// Iterate over every light, calculating its contribution the color of ISEC.
// BRDF is used to calculate the actual effect; COLOR is the "base color"
//
Color
Scene::illum (const Intersect &isec, const Color &color, const Brdf &brdf,
	      TraceState &tstate)
  const
{
  LightSamples &lsamples = tstate.global.light_samples;

  lsamples.generate (isec, color, brdf, lights, tstate);

  stats.illum_calls++;
  stats.illum_samples += lsamples.size ();

  // The maximum _possible_ output radiance (from lights), if all remaining
  // samples are not shadowed.
  //
  Color poss_radiance;
  for (LightSamples::iterator s = lsamples.begin (); s != lsamples.end (); s++)
    poss_radiance += s->val;

  // Accumulated output radiance
  //
  Color radiance;

  // Now shoot shadow rays to check the visibility of the chosen set of
  // light samples, accumulating the resulting radiance in RADIANCE.

  TraceState &sub_tstate
    = tstate.subtrace_state (TraceState::SHADOW, isec.surface);

  for (LightSamples::iterator s = lsamples.begin (); s != lsamples.end (); s++)
    if (s->val > 0)
      {
	// If RADIANCE is beyond some threshold, give up even though we
	// haven't finished all samples yet.  This means that in cases
	// where the output radiance is dominated by very bright lights,
	// we won't waste time calculating the remaining dim ones.
	//
	if (radiance > poss_radiance * 0.95 /* XXX make a variable */)
	  {
	    // XXX somehow use the unused samples, except without sending out
	    // shadow rays [perhaps (1) keep track of last-known visibility
	    // per-light, and (2) update the visibility of some random subset
	    // of the remaining unused lights]
	    //
	    break;
	  }

	const Ray shadow_ray (isec.point, s->dir, s->dist);

	// Find any surface that's shadowing LIGHT_RAY.
	//
	const Surface *shadower
	  = sub_tstate.shadow_caster (shadow_ray, *s->light, isec);

	if (! shadower)
	  //
	  // The surface is not shadowed at all, just add the light.
	  //
	  radiance += s->val;

	else if (shadower->shadow_type != Material::SHADOW_OPAQUE)
	  //
	  // There's a shadower, but it's not opaque, so give it (and
	  // any further surfaces) a chance to attentuate the color.
	  {
	    stats.scene_slow_shadow_traces++;

	    // The reflected radiance from this sample, after being adjusted
	    // for the filtering of the source irradiance through SHADOWER.
	    //
	    Color filtered = sub_tstate.shadow (shadow_ray, s->val, *s->light);

	    // Add the final filtered radiance to RADIANCE, and also
	    // adjust our estimation of the total possible radiance to
	    // account for the filtering.
	    //
	    radiance += filtered;
	    poss_radiance -= s->val - filtered;
	  }

	else
	  // The surface is shadowed; subtract this light from the
	  // possible radiance.
	  //
	  poss_radiance -= s->val;
      }

  return radiance;
}



// Returns the background color in the direction pointed to by RAY
//
Color
Scene::background (const Ray &ray) const
{
  if (bg_cube)
    return bg_cube->map (ray.dir);
  else
    return bg_color;
}

void
Scene::set_background (const Color &col)
{
  if (bg_cube)
    {
      delete bg_cube;
      bg_cube = 0;
    }

  bg_color = col;
}

void
Scene::set_background (const Cubetex *cube)
{
  delete bg_cube;

  bg_cube = cube;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
