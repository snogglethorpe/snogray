// sphere-isec.h -- Sphere intersection
//
//  Copyright (C) 2005, 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_ISEC_H__
#define __SPHERE_ISEC_H__

#include "pos.h"
#include "vec.h"
#include "ray.h"
#include "quadratic-roots.h"


namespace snogray {


// Return true if a sphere centered at CENTER with radius RADIUS is
// intersected by an infinite ray from RAY_ORIGIN in direction
// RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.  Only intersections with a parameter distance of
// MIN_T or greater are considered.
//
static inline bool
sphere_intersects (const Pos &center, dist_t radius,
		   const Pos &ray_origin, const Vec &ray_dir,
		   dist_t min_t, dist_t &t)
{
  Vec ray_origin_offs = ray_origin - center;

  // Coefficients of the quadratic equation we'll solve.
  //
  dist_t a = ray_dir.length_squared ();
  dist_t b = 2 * dot (ray_dir, ray_origin_offs);
  dist_t c = ray_origin_offs.length_squared () - radius * radius;

  // Compute intersection points.
  //
  dist_t roots[2];
  unsigned nroots = quadratic_roots (a, b, c, roots);
  for (unsigned i = 0; i < nroots; i++)
    {
      t = roots[i];
      if (t > min_t)
	return true;
    }

  return false;
}

// Return true if a sphere centered at CENTER with radius RADIUS is
// intersected by an infinite ray from RAY_ORIGIN in direction
// RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.
//
static inline bool
sphere_intersects (const Pos &center, dist_t radius,
		   const Pos &ray_origin, const Vec &ray_dir,
		   dist_t &t)
{
  return sphere_intersects (center, radius, ray_origin, ray_dir, 0, t);
}


// Return true if a sphere centered at CENTER with radius RADIUS is
// intersected by RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY's dir field required to reach the intersection
// point from RAY's origin.
//
static inline bool
sphere_intersects (const Pos &center, dist_t radius, const Ray &ray, dist_t &t)
{
  if (sphere_intersects (center, radius, ray.origin, ray.dir, ray.t0, t))
    return t < ray.t1;
  return false;
}


}

#endif // __SPHERE_ISEC_H__


// arch-tag: 032d0cc1-ccae-4dfe-8806-e6dbdaee0cce
