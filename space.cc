// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011  Miles Bader <miles@gnu.org>
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
  IntersectsCallback (const Ray &_ray, RenderContext &_context)
    : ray (_ray), intersects (false), context (_context)
  { }

  virtual bool operator() (const Surface *surf)
  {
    intersects = surf->intersects (ray, context);

    if (intersects)
      // We can immediately return it; stop looking any further.
      //
      stop_iteration ();

    return intersects;
  }

  const Ray &ray;

  // True if we found an intersecting object.
  //
  bool intersects;

  RenderContext &context;
};


// Return true if any object intersects RAY.
//
bool
Space::intersects (const Ray &ray, RenderContext &context) const
{
  IntersectsCallback intersects_cb (ray, context);

  for_each_possible_intersector (ray, intersects_cb, context,
				 context.stats.shadow);

  return intersects_cb.intersects;
}



// Occludes calculation, including partial occlusion.

struct OccludesCallback : Space::IntersectCallback
{
  OccludesCallback (const Ray &_ray, const Medium &_medium,
		    Color &_total_transmittance,
		    RenderContext &_context)
    : ray (_ray), total_transmittance (_total_transmittance),
      medium (_medium), context (_context), occludes (false)
  { }

  virtual bool operator() (const Surface *surf)
  {
    occludes = surf->occludes (ray, medium, total_transmittance, context);
    if (occludes)
      stop_iteration ();
    return occludes;
  }

  const Ray &ray;

  // Product of all surfaces encountered so far.
  //
  Color &total_transmittance;

  // Medium in which to evaluate material occlusion.
  //
  const Medium &medium;

  RenderContext &context;

  // True if we found a totally-occluding object.
  //
  bool occludes;
};


// Return true if some surface in this space completely occludes RAY.
// If no surface completely occludes RAY, then return false, and
// multiply TOTAL_TRANSMITTANCE by the transmittance of any surfaces
// in this space which partially occlude RAY, evaluated in medium
// MEDIUM.
//
// Note that this method does not try to handle non-trivial forms of
// transparency/translucency (for instance, a "glass" material is
// probably considered opaque because it changes light direction as
// well as transmitting it), nor does it deal with anything except
// surfaces.
//
bool
Space::occludes (const Ray &ray, const Medium &medium,
		 Color &total_transmittance,
		 RenderContext &context)
  const
{
  OccludesCallback occludes_cb (ray, medium, total_transmittance, context);

  for_each_possible_intersector (ray, occludes_cb, context,
				 context.stats.shadow);

  return occludes_cb.occludes;
}


// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
