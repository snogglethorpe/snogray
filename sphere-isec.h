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
template<typename T>
bool
sphere_intersects (const TPos<T> &center, T radius,
		   const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		   T min_t, T &t)
{
  TVec<T> ray_origin_offs = ray_origin - center;
  T dir_diff = dot (ray_dir, ray_origin_offs);
  T dir_dir = dot (ray_dir, ray_dir); // theoretically, exactly 1; in
				// practice, not _quite_
  T diff_diff = dot (ray_origin_offs, ray_origin_offs);
  T determ
    = ((dir_diff * dir_diff) - dir_dir * (diff_diff - (radius * radius)));

  if (determ >= 0)
    {
      t = -dir_diff / dir_dir;

      if (determ > 0)
	{
	  T determ_factor = sqrt (determ) / dir_dir;

	  // See which of T-DETERM_FACTOR or T+DETERM_FACTOR is greater
	  // than MIN_T.
	  //
	  t -= determ_factor;	// first try -DETERM_FACTOR
	  if (t < min_t)
	    t += determ_factor + determ_factor; // then try +DETERM_FACTOR
	}

      return t >= min_t;
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
template<typename T>
bool
sphere_intersects (const TPos<T> &center, T radius,
		   const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		   T &t)
{
  return sphere_intersects (center, radius, ray_origin, ray_dir, T(0), t);
}


// Return true if a sphere centered at CENTER with radius RADIUS is
// intersected by the ray RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.
//
template<typename T>
bool
sphere_intersects (const TPos<T> &center, T radius, const TRay<T> &ray, T &t)
{
  if (sphere_intersects (center, radius, ray.origin, ray.dir, ray.t0, t))
    return t < ray.t1;
  return false;
}


}

#endif // __SPHERE_ISEC_H__


// arch-tag: 032d0cc1-ccae-4dfe-8806-e6dbdaee0cce
