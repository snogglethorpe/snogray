// sphere.cc -- Sphere object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

inline dist_t
Sphere::intersection_distance (const Ray &ray, bool closest_ok) const
{
  Vec dir = ray.dir;		// must be a unit vector
  Vec diff = ray.origin - center;
  double dir_diff = dir.dot (diff);
  double dir_dir =  dir.dot (dir);
  double determ
    = (dir_diff * dir_diff) - dir_dir * (diff.dot(diff) - (radius * radius));

  if (determ > -Eps)
    {
      double common = -dir_diff / dir_dir;

      if (determ < Eps && common > Eps)
	{
	  if (closest_ok)
	    return common;
	}
      else
	{
	  double determ_factor = sqrtf (determ) / dir_dir;
	  double t0 = common - determ_factor;
	  double t1 = common + determ_factor;

	  if (closest_ok && t0 > Eps)
	    return t0;
	  else if (t1 > Eps)
	    return t1;
	}
    }

  return 0;
}

// Return the distance from RAY's origin of the closest intersection of
// this object with RAY, or 0 if there is none.
//
dist_t
Sphere::intersection_distance (const Ray &ray) const
{
  return intersection_distance (ray, true);
}

// Given that RAY's origin is known to lie on this object, return the
// distance from RAY's origin to the _next_ closest intersection of this
// object with RAY, or 0 if there is none.  For non-convex objects such
// as triangles, the default implementation which always returns 0 is
// correct.
//
dist_t
Sphere::next_intersection_distance (const Ray &ray) const
{
  return intersection_distance (ray, false);
}

Vec
Sphere::normal (const Pos &point, const Vec &incoming) const
{
  return (point - center).unit ();
}

// Return a bounding box for this object.
BBox
Sphere::bbox () const
{
  return BBox (Pos (center.x - radius, center.y - radius, center.z - radius),
	       Pos (center.x + radius, center.y + radius, center.z + radius));
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
