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

dist_t
Sphere::intersection_distance (const Ray &ray) const
{
  Vec dir = ray.dir;		// must be a unit vector
  Vec diff = ray.origin - center;
  float dir_diff = dir.dot (diff);
  float dir_dir =  dir.dot (dir);
  float determ = dir_diff*dir_diff - dir_dir * (diff.dot(diff) - radius*radius);

  if (determ >= 0)
    {
      float common = -dir_diff / dir_dir;

      if (determ == 0 && common > 0)
	return common;
      else
	{
	  float determ_factor = sqrtf (determ) / dir_dir;
	  float t0 = common - determ_factor;
	  float t1 = common + determ_factor;

	  if (t0 > 0)
	    return t0;
	  else if (t1 > 0)
	    return t1;
	}
    }

  return 0;
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
