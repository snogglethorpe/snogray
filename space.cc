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
#include "shadow-ray.h"

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
  ClosestIntersectCallback (Ray &_ray, const Surface::IsecCtx &_isec_ctx)
    : ray (_ray), closest (0), isec_ctx (_isec_ctx),
      surf_isec_tests (0), surf_isec_hits (0),
      neg_cache_hits (0), neg_cache_collisions (0)
  { }

  virtual void operator() (Surface *);

  Ray &ray;

  // Information about the closest intersection we've found
  //
  const Surface::IsecInfo *closest;

  const Surface::IsecCtx &isec_ctx;

  unsigned surf_isec_tests, surf_isec_hits;
  unsigned neg_cache_hits, neg_cache_collisions;
};

void
ClosestIntersectCallback::operator () (Surface *surf)
{
  Trace &trace = isec_ctx.trace;

  if (! trace.negative_isec_cache.contains (surf))
    {
      const Surface::IsecInfo *isec_info = surf->intersect (ray, isec_ctx);
      if (isec_info)
	{
	  closest = isec_info;
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

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Space::intersect (Ray &ray, const Surface::IsecCtx &isec_ctx) const
{
  // A callback which is called for each surface in this space
  // that may intersect RAY.
  //
  ClosestIntersectCallback closest_isec_cb (ray, isec_ctx);

  Trace &trace = isec_ctx.trace;

  trace.negative_isec_cache.clear ();

  if (trace.horizon_hint)
    trace.negative_isec_cache.add (trace.horizon_hint);

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
  ShadowCallback (const ShadowRay &_ray, Trace &_trace,
		  const Light *_hint_light)
    : ray (_ray),
      shadow_type (Material::SHADOW_NONE),
      trace (_trace), hint_light (_hint_light),
      surf_isec_tests (0), surf_isec_hits (0),
      neg_cache_hits (0), neg_cache_collisions (0)
  { }

  virtual void operator() (Surface *);

  const ShadowRay &ray;

  // Strongest type of shadow discovered.
  //
  Material::ShadowType shadow_type;

  Trace &trace;

  const Light *hint_light;

  unsigned surf_isec_tests, surf_isec_hits;
  unsigned neg_cache_hits, neg_cache_collisions;
};

void
ShadowCallback::operator () (Surface *surface)
{
  if (! trace.negative_isec_cache.contains (surface))
    {
      Material::ShadowType stype = surface->shadow (ray);

      if (stype > shadow_type)
	{
	  shadow_type = stype;

	  if (stype == Material::SHADOW_OPAQUE)
	    {
	      // Remember which surface we found, so we can try it first
	      // next time.
	      //
	      if (hint_light)
		trace.shadow_hints[hint_light->num] = surface;

	      // A simple opaque surface blocks everything; we can
	      // immediately return it; stop looking any further.
	      //
	      stop_iteration ();
	    }
	}

      if (stype != Material::SHADOW_NONE)
	surf_isec_hits++;
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


// Return the strongest type of shadowing effect any object in this space
// has on RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
// If HINT_LIGHT is non-zero, then the shadow-hint entry for HINT_LIGHT
// should be updated to hold the first object which results in an opaque
// shadow.
//
Material::ShadowType
Space::shadow (const ShadowRay &ray, Trace &trace, const Light *hint_light)
  const
{
  trace.negative_isec_cache.clear ();

  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  if (ray.isec.no_self_shadowing)
    trace.negative_isec_cache.add (ray.isec.surface);

  ShadowCallback shadow_cb (ray, trace, hint_light);

  for_each_possible_intersector (ray, shadow_cb);

  TraceStats::IsecStats &isec_stats = trace.global.stats.shadow;
  isec_stats.surface_intersects_tests	+= shadow_cb.surf_isec_tests;
  isec_stats.surface_intersects_hits	+= shadow_cb.surf_isec_hits;
  isec_stats.neg_cache_hits		+= shadow_cb.neg_cache_hits;
  isec_stats.neg_cache_collisions	+= shadow_cb.neg_cache_collisions;
  isec_stats.space_node_intersect_calls += shadow_cb.node_intersect_calls;

  return shadow_cb.shadow_type;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
