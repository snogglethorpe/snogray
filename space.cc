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
// with a ray, information about the closest intersection)

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


// Simple (boolean) intersection testing

struct IntersectsCallback : Space::IntersectCallback
{
  IntersectsCallback (const ShadowRay &_ray, RenderContext &_context,
		      const Surface *_reject = 0)
    : ray (_ray), intersects (false),
      context (_context), reject (_reject)
  { }

  virtual bool operator() (const Surface *surf)
  {
    if (surf == reject)
      return false;

    intersects = surf->intersects (ray, context);

    if (intersects)
      // We can immediately return it; stop looking any further.
      //
      stop_iteration ();

    return intersects;
  }

  const ShadowRay &ray;

  // True if we found an intersecting object.
  //
  bool intersects;

  RenderContext &context;

  // If non-zero, this surface is always immediately rejected.
  //
  const Surface *reject;
};


// Return true if any object intersects RAY.
//
bool
Space::intersects (const ShadowRay &ray, RenderContext &context) const
{
  // If possible, prime the negative intersect cache with the current
  // surface, to avoid wasting time test it for intersection.
  //
  const Surface *reject = ray.isec.no_self_shadowing;

  IntersectsCallback intersects_cb (ray, context, reject);

  for_each_possible_intersector (ray, intersects_cb, context,
				 context.stats.shadow);

  return intersects_cb.intersects;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
