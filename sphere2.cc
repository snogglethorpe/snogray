// sphere2.cc -- Alternative sphere surface
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
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
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Sphere2::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  Ray oray = world_to_local (ray);
  
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);
  if (t > oray.t0 && t < oray.t1)
    {
      ray.t1 = t;
      return new (isec_ctx) IsecInfo (ray, this, Vec (oray.extension (t)));
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

  return Intersect (ray, sphere, Frame (point, s, t, norm), trace);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Sphere2::shadow (const ShadowRay &sray) const
{
  Ray oray = world_to_local (sray.as_ray ());
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);

  if (t > oray.t0 && t < oray.t1)
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}


// arch-tag: 4eacb938-b015-4a82-936b-effbdc72eab5
