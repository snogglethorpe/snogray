// sphere2.cc -- Alternative sphere surface
//
//  Copyright (C) 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "sphere-isec.h"
#include "intersect.h"
#include "shadow-ray.h"

#include "sphere2.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with CONTEXT); otherwise return zero.
//
const Surface::IsecInfo *
Sphere2::intersect (Ray &ray, RenderContext &context) const
{
  Ray oray = world_to_local (ray);
  
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);
  if (t > oray.t0 && t < oray.t1)
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, this, Vec (oray.extension (t)));
    }
  else
    return false;
}

// Create an Intersect object for this intersection.
//
Intersect
Sphere2::IsecInfo::make_intersect (Trace &trace) const
{
  Pos point = ray.end ();

  // Choose the second tangent vector (perpendicular to ONORM) in object
  // space, OT (this is convenient for later calculating the first tangent
  // vector in world space -- it will then point towards the north pole).
  //
  // We try to make OT point "around" the sphere, by calculating it as
  // the cross product of ONORM and an "up" vector (0,0,1).  However if
  // ONORM itself is (0,0,1) or (0,0,-1), we can't do that; in that
  // case, we choose an arbitrary vector for OT instead.
  //
  Vec ot;
  if (abs (onorm.x) < Eps && abs (onorm.y) < Eps)
    ot = Vec (1, 0, 0);
  else
    ot = cross (onorm, Vec (0, 0, 1));

  // Calculate the normal and tangent vectors in world space.  NORM and
  // T are just ONORM and OT converted from the local coordinate system
  // to world space, and S is just the cross product of NORM and T.
  //
  Vec norm = sphere->normal_to_world (onorm).unit ();
  Vec t = sphere->local_to_world (ot).unit ();
  Vec s = cross (norm, t);

  // Calculate texture coords.  Note that ONORM is also the intersection
  // location in object coordinates.
  //
  UV tex_coords (atan2 (onorm.y, onorm.x) * INV_PIf * 0.5f + 0.5f,
		 asin (clamp (onorm.z, -1.f, 1.f)) * INV_PIf + 0.5f);

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  UV dTds (INV_PIf * 0.5f, 0), dTdt (0, INV_PIf);

  return Intersect (ray, trace, sphere, Frame (point, s, t, norm),
		    tex_coords, dTds, dTdt);
}

// Return true if this surface intersects RAY.
//
bool
Sphere2::intersects (const ShadowRay &sray, RenderContext &) const
{
  Ray oray = world_to_local (sray.as_ray ());
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);
  return (t > oray.t0 && t < oray.t1);
}


// arch-tag: 4eacb938-b015-4a82-936b-effbdc72eab5
