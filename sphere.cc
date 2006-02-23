// sphere.cc -- Sphere surface
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "sphere.h"

#include "intersect.h"

using namespace Snogray;

// Return the distance from RAY's origin to the closest intersection
// of this surface with RAY, or 0 if there is none.  RAY is considered
// to be unbounded.
//
// If intersection succeeds, then ISEC_PARAMS is updated with other
// (surface-specific) intersection parameters calculated.
//
// NUM is which intersection to return, for non-flat surfaces that may
// have multiple intersections -- 0 for the first, 1 for the 2nd, etc
// (flat surfaces will return failure for anything except 0).
//
dist_t
Sphere::intersection_distance (const Ray &ray,
			       IsecParams &isec_params,
			       unsigned num)
  const
{
  if (num > 1)
    return 0;

  Vec dir = ray.dir;		  // must be a unit vector
  double dir_dir = dot (dir, dir); // theoretically, exactly 1; in
				  // practice, not _quite_
  Vec diff = ray.origin - center;
  double dir_diff = dot (dir, diff);
  double determ
    = (dir_diff * dir_diff) - dir_dir * (dot (diff, diff) - (radius * radius));

  if (determ > -Eps)
    {
      double common = -dir_diff / dir_dir;

      if (determ < Eps && common > Eps)
	{
	  if (num == 0)
	    return common;
	}
      else
	{
	  double determ_factor = sqrt (determ) / dir_dir;
	  double t0 = common - determ_factor;
	  double t1 = common + determ_factor;

	  if (num == 0 && t0 > Eps)
	    return t0;
	  else if (t1 > Eps)
	    return t1;
	}
    }

  return 0;
}

Intersect
Sphere::intersect_info (const Ray &ray, const IsecParams &isec_params)
  const
{
  Pos point = ray.end ();
  return Intersect (ray, this, point, (point - center).unit ());
}

// Return a bounding box for this surface.
BBox
Sphere::bbox () const
{
  return BBox (Pos (center.x - radius, center.y - radius, center.z - radius),
	       Pos (center.x + radius, center.y + radius, center.z + radius));
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
