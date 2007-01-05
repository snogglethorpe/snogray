// sphere-isec.h -- Sphere intersection
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_ISEC_H__
#define __SPHERE_ISEC_H__

#include "pos.h"
#include "vec.h"

namespace snogray {


// Any intersection closer than this will be rejected as spurious
// (i.e. logically zero, but off due to floating-point imprecision).
//
#define MIN_SPHERE_ISEC_DIST (1e-7)


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
		  bool far_isec = false)
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

      if (determ <= 0 && common > 0)
	{
	  if (! far_isec)
	    return common;
	}
      else
	{
	  T determ_factor = sqrt (determ) / dir_dir;
	  T t0 = common - determ_factor;
	  T t1 = common + determ_factor;

	  if (!far_isec && t0 > MIN_SPHERE_ISEC_DIST)
	    return t0;
	  else if (t1 > MIN_SPHERE_ISEC_DIST)
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
		  bool far_isec = false)
{
  return sphere_intersect<T> (radius, ray_origin - center, ray_dir, far_isec);
}


}

#endif /* __SPHERE_ISEC_H__ */

// arch-tag: 032d0cc1-ccae-4dfe-8806-e6dbdaee0cce
