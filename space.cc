// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface.h"
#include "light.h"

#include "space.h"


using namespace snogray;


// This is located here because putting it in "space.h" causes recursive
// include problems (because "space.h" then must include "surface.h").
//
void
Space::add (Surface *surface)
{
  add (surface, surface->bbox ());
}


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct ClosestIntersectCallback : Space::IntersectCallback
{
  ClosestIntersectCallback (Ray &_ray, Trace &_trace)
    : ray (_ray), closest (0), isec_ctx (_trace), trace (_trace),
      surf_isec_tests (0), surf_isec_hits (0),
      neg_cache_hits (0), neg_cache_collisions (0)
  { }

  virtual void operator() (Surface *);

  Ray &ray;

  // Information about the closest intersection we've found
  //
  const Surface::IsecInfo *closest;

  Surface::IsecCtx isec_ctx;

  Trace &trace;

  unsigned surf_isec_tests, surf_isec_hits;
  unsigned neg_cache_hits, neg_cache_collisions;
};

void
ClosestIntersectCallback::operator () (Surface *surf)
{
  if (surf != trace.horizon_hint)
    {
      if (! trace.negative_isec_cache.contains (surf))
	{
	  Surface::IsecInfo *isec_info = surf->intersect (ray, isec_ctx);
	  if (isec_info)
	    {
	      closest = isec_info;
	      trace.horizon_hint = surf;
	      surf_isec_hits++;
	    }
	  else
	    {
	      bool collision = trace.negative_isec_cache.add (surf);
	      if (collision)
		neg_cache_collisions++;
	    }

	  surf_isec_tests++;
	}
      else
	neg_cache_hits++;
    }
}

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Space::intersect (Ray &ray, Trace &trace) const
{
  // Make a callback, and call it for each surface that may intersect
  // the ray.

  ClosestIntersectCallback closest_isec_cb (ray, trace);

  // If there's a horizon hint, try to use it to reduce the horizon
  // before searching -- space searching can dramatically improve given
  // a limited search space.
  //
  const Surface *hint = trace.horizon_hint;
  if (hint)
    {
      Surface::IsecInfo *isec_info
	= hint->intersect (ray, closest_isec_cb.isec_ctx);
      if (isec_info)
	{
	  closest_isec_cb.closest = isec_info;
	  trace.global.stats.horizon_hint_hits++;
	}
      else
	{
	  trace.horizon_hint = 0; // clear the hint
	  trace.global.stats.horizon_hint_misses++;
	}
    }

  trace.negative_isec_cache.clear ();

  for_each_possible_intersector (ray, closest_isec_cb);

  TraceStats::IsecStats &isec_stats = trace.global.stats.intersect;
  isec_stats.surface_intersects_tests	+= closest_isec_cb.surf_isec_tests;
  isec_stats.surface_intersects_hits	+= closest_isec_cb.surf_isec_hits;
  isec_stats.neg_cache_hits		+= closest_isec_cb.neg_cache_hits;
  isec_stats.neg_cache_collisions	+= closest_isec_cb.neg_cache_collisions;
  isec_stats.space_node_intersect_calls += closest_isec_cb.node_intersect_calls;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct ShadowCallback : Space::IntersectCallback
{
  ShadowCallback (const Ray &_light_ray, const Intersect &_isec,
		  Trace &_trace, const Light *_light)
    : light_ray (_light_ray), isec (_isec),
      shadower (0), trace (_trace), light (_light),
      surf_isec_tests (0), surf_isec_hits (0),
      neg_cache_hits (0), neg_cache_collisions (0)
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

  unsigned surf_isec_tests, surf_isec_hits;
  unsigned neg_cache_hits, neg_cache_collisions;
};

void
ShadowCallback::operator () (Surface *surface)
{
  Material::ShadowType shadow_type = surface->material->shadow_type;

  if (surface != isec.surface && shadow_type != Material::SHADOW_NONE)
    {
      if (! trace.negative_isec_cache.contains (surface))
	{
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

	      surf_isec_hits++;
	    }
	  else
	    {
	      bool collision = trace.negative_isec_cache.add (surface);
	      if (collision)
		neg_cache_collisions++;
	    }

	  surf_isec_tests++;
	}
      else
	neg_cache_hits++;
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
Space::shadow_caster (const Ray &light_ray, const Intersect &isec,
		      Trace &trace, const Light *light)
  const
{
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

  ShadowCallback shadow_cb (light_ray, isec, trace, light);

  trace.negative_isec_cache.clear ();

  for_each_possible_intersector (light_ray, shadow_cb);

  TraceStats::IsecStats &isec_stats = trace.global.stats.shadow;
  isec_stats.surface_intersects_tests	+= shadow_cb.surf_isec_tests;
  isec_stats.surface_intersects_hits	+= shadow_cb.surf_isec_hits;
  isec_stats.neg_cache_hits		+= shadow_cb.neg_cache_hits;
  isec_stats.neg_cache_collisions	+= shadow_cb.neg_cache_collisions;
  isec_stats.space_node_intersect_calls += shadow_cb.node_intersect_calls;

  return shadow_cb.shadower;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
