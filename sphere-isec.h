// sphere-isec.h -- Sphere intersection
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

namespace snogray {


// If a ray from RAY_ORIGIN with direction RAY_DIR (which must be a unit
// vector) intersects a sphere centered at the origin with radius RADIUS,
// then return the distance to the intersection.  If the ray doesn't
// intersect, return 0.  In the case where there are two possible
// intersections, the nearest intersection is returned unless FAR_ISEC is
// true.
//
template<typename T>
inline T
sphere_intersect (T radius,
		  const TVec<T> &ray_origin_offs, const TVec<T> &ray_dir,
		  T min_dist = 0)
{
  T dir_diff = dot (ray_dir, ray_origin_offs);
  T dir_dir = dot (ray_dir, ray_dir); // theoretically, exactly 1; in
				// practice, not _quite_
  T diff_diff = dot (ray_origin_offs, ray_origin_offs);
  T determ
    = ((dir_diff * dir_diff) - dir_dir * (diff_diff - (radius * radius)));

  if (determ >= 0)
    {
      T common = -dir_diff / dir_dir;

      if (determ <= 0)
	{
	  if (common > min_dist)
	    return common;
	}
      else
	{
	  T determ_factor = sqrt (determ) / dir_dir;
	  T t0 = common - determ_factor;
	  T t1 = common + determ_factor;

	  if (t0 > min_dist)
	    return t0;
	  else
	    return t1;
	}
    }

  return 0;
}


// If a ray from RAY_ORIGIN with direction RAY_DIR (which must be a unit
// vector) intersects a sphere centered at CENTER with radius RADIUS, then
// return the distance to the intersection.  If the ray doesn't intersect,
// return 0.  In the case where there are two possible intersections, the
// nearest intersection is returned unless FAR_ISEC is true.
//
template<typename T>
inline T
sphere_intersect (const TPos<T> &center, T radius,
		  const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		  T min_dist = 0)
{
  return sphere_intersect<T> (radius, ray_origin - center, ray_dir, min_dist);
}


}

#endif /* __SPHERE_ISEC_H__ */

// arch-tag: 032d0cc1-ccae-4dfe-8806-e6dbdaee0cce
