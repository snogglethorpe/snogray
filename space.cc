// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface.h"
#include "light.h"
#include "shadow-ray.h"
#include "trace-cache.h"

#include "space.h"


using namespace snogray;


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct ClosestIntersectCallback : Space::IntersectCallback
{
  ClosestIntersectCallback (Ray &_ray, const Surface::IsecCtx &_isec_ctx,
			    const Surface *_reject = 0)
    : ray (_ray), closest (0), isec_ctx (_isec_ctx), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf != reject)
      {
	const Surface::IsecInfo *isec_info = surf->intersect (ray, isec_ctx);
	if (isec_info)
	  {
	    closest = isec_info;
	    return true;
	  }
      }

    return false;
  }


  Ray &ray;

  // Information about the closest intersection we've found
  //
  const Surface::IsecInfo *closest;

  const Surface::IsecCtx &isec_ctx;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Space::intersect (Ray &ray, const Surface::IsecCtx &isec_ctx) const
{
  TraceContext &context = isec_ctx.context;
  TraceCache &cache = isec_ctx.cache;

  // A callback which is called for each surface in this space
  // that may intersect RAY.
  //
  ClosestIntersectCallback closest_isec_cb (ray, isec_ctx, cache.horizon_hint);

  for_each_possible_intersector (ray, closest_isec_cb, context,
				 context.stats.intersect);

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct ShadowCallback : Space::IntersectCallback
{
  ShadowCallback (const ShadowRay &_ray, const Surface::IsecCtx &_isec_ctx,
		  const Light *_hint_light, const Surface *_reject = 0)
    : ray (_ray),
      shadow_type (Material::SHADOW_NONE),
      isec_ctx (_isec_ctx), hint_light (_hint_light), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf == reject)
      return false;

    Material::ShadowType stype = surf->shadow (ray, isec_ctx);

    if (stype > shadow_type)
      {
	shadow_type = stype;

	if (stype == Material::SHADOW_OPAQUE)
	  {
	    // Remember which surface we found, so we can try it first
	    // next time.
	    //
	    if (hint_light)
	      isec_ctx.cache.shadow_hints[hint_light->num] = surf;

	    // A simple opaque surface blocks everything; we can
	    // immediately return it; stop looking any further.
	    //
	    stop_iteration ();
	  }
      }

    return stype != Material::SHADOW_NONE;
  }

  const ShadowRay &ray;

  // Strongest type of shadow discovered.
  //
  Material::ShadowType shadow_type;

  const Surface::IsecCtx &isec_ctx;

  const Light *hint_light;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


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
Space::shadow (const ShadowRay &ray, const Surface::IsecCtx &isec_ctx,
	       const Light *hint_light)
  const
{
  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  const Surface *reject = ray.isec.no_self_shadowing ? ray.isec.surface : 0;

  ShadowCallback shadow_cb (ray, isec_ctx, hint_light, reject);

  for_each_possible_intersector (ray, shadow_cb, isec_ctx.context,
				 isec_ctx.context.stats.shadow);

  return shadow_cb.shadow_type;
}


// Simple shadow intersection testing

struct SimpleShadowCallback : Space::IntersectCallback
{
  SimpleShadowCallback (const ShadowRay &_ray, Surface::IsecCtx &_isec_ctx,
			const Light *_hint_light, const Surface *_reject = 0)
    : ray (_ray), shadows (false),
      isec_ctx (_isec_ctx), hint_light (_hint_light), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf == reject)
      return false;

    shadows = (surf->shadow (ray, isec_ctx) != Material::SHADOW_NONE);

    if (shadows)
      {
	// Remember which surface we found, so we can try it first
	// next time.
	//
	if (hint_light)
	  isec_ctx.cache.shadow_hints[hint_light->num] = surf;

	// We can immediately return it; stop looking any further.
	//
	stop_iteration ();
      }

    return shadows;
  }

  const ShadowRay &ray;

  // True if we found a shadowing object.
  //
  bool shadows;

  Surface::IsecCtx &isec_ctx;

  const Light *hint_light;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


// Return true if any object intersects RAY.
//
// If HINT_LIGHT is non-zero, then the shadow-hint entry for HINT_LIGHT
// should be updated to hold the first object which results in an opaque
// shadow.
//
bool
Space::shadows (const ShadowRay &ray, Surface::IsecCtx &isec_ctx,
		const Light *hint_light)
  const
{
  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  const Surface *reject = ray.isec.no_self_shadowing ? ray.isec.surface : 0;

  SimpleShadowCallback shadow_cb (ray, isec_ctx, hint_light, reject);

  for_each_possible_intersector (ray, shadow_cb, isec_ctx.context,
				 isec_ctx.context.stats.shadow);

  return shadow_cb.shadows;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
