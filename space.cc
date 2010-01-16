// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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

#include "space.h"


using namespace snogray;


// "Closest" intersection testing (tests all surfaces for intersection
// with a ray, returns the distance to the closest intersection)

struct ClosestIntersectCallback : Space::IntersectCallback
{
  ClosestIntersectCallback (Ray &_ray, RenderContext &_context)
    : ray (_ray), closest (0), context (_context)
  { }

  virtual bool operator() (const Surface *surf)
  {
    const Surface::IsecInfo *isec_info = surf->intersect (ray, context);
    if (isec_info)
      {
	closest = isec_info;
	return true;
      }

    return false;
  }


  Ray &ray;

  // Information about the closest intersection we've found
  //
  const Surface::IsecInfo *closest;

  RenderContext &context;
};


// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Space::intersect (Ray &ray, RenderContext &context) const
{
  // A callback which is called for each surface in this space
  // that may intersect RAY.
  //
  ClosestIntersectCallback closest_isec_cb (ray, context);

  for_each_possible_intersector (ray, closest_isec_cb, context,
				 context.stats.intersect);

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct ShadowCallback : Space::IntersectCallback
{
  ShadowCallback (const ShadowRay &_ray, RenderContext &_context,
		  const Surface *_reject = 0)
    : ray (_ray),
      shadow_type (Material::SHADOW_NONE),
      context (_context), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf == reject)
      return false;

    Material::ShadowType stype = surf->shadow (ray, context);

    if (stype > shadow_type)
      {
	shadow_type = stype;

	if (stype == Material::SHADOW_OPAQUE)
	  // A simple opaque surface blocks everything; we can
	  // immediately return it; stop looking any further.
	  //
	  stop_iteration ();
      }

    return stype != Material::SHADOW_NONE;
  }

  const ShadowRay &ray;

  // Strongest type of shadow discovered.
  //
  Material::ShadowType shadow_type;

  RenderContext &context;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


// Return the strongest type of shadowing effect any object in this space
// has on RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Space::shadow (const ShadowRay &ray, RenderContext &context) const
{
  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  const Surface *reject = ray.isec.no_self_shadowing ? ray.isec.surface : 0;

  ShadowCallback shadow_cb (ray, context, reject);

  for_each_possible_intersector (ray, shadow_cb, context, context.stats.shadow);

  return shadow_cb.shadow_type;
}


// Simple shadow intersection testing

struct SimpleShadowCallback : Space::IntersectCallback
{
  SimpleShadowCallback (const ShadowRay &_ray, RenderContext &_context,
			const Surface *_reject = 0)
    : ray (_ray), shadows (false),
      context (_context), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf == reject)
      return false;

    shadows = (surf->shadow (ray, context) != Material::SHADOW_NONE);

    if (shadows)
      // We can immediately return it; stop looking any further.
      //
      stop_iteration ();

    return shadows;
  }

  const ShadowRay &ray;

  // True if we found a shadowing object.
  //
  bool shadows;

  RenderContext &context;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


// Return true if any object intersects RAY.
//
bool
Space::shadows (const ShadowRay &ray, RenderContext &context) const
{
  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  const Surface *reject = ray.isec.no_self_shadowing ? ray.isec.surface : 0;

  SimpleShadowCallback shadow_cb (ray, context, reject);

  for_each_possible_intersector (ray, shadow_cb, context, context.stats.shadow);

  return shadow_cb.shadows;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
