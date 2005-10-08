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

#include <iostream>

#include "sphere.h"

#include "intersect.h"

Space::dist_t
Sphere::intersection_distance (const Ray &ray) const
{
  Vec dir = ray.dir;		// must be a unit vector
  Vec diff = ray.origin - center;
  float dir_diff = dir.dot (diff);
  float dir_dir =  dir.dot (dir);
  float determ = dir_diff*dir_diff - dir_dir * (diff.dot(diff) - radius*radius);

//   std::cout << "Sphere::intersection_distance (" << ray << "):" << std::endl;
//   std::cout << "    dir      = " << dir << std::endl;
//   std::cout << "    diff     = " << diff << std::endl;
//   std::cout << "    dir_diff = " << dir_diff << std::endl;
//   std::cout << "    dir_dir  = " << dir_dir << std::endl;
//   std::cout << "    determ   = " << determ << std::endl;

  if (determ >= 0)
    {
      float common = -dir_diff / dir_dir;

//       std::cout << "    common   = " << common << std::endl;

      if (determ == 0 && common > 0)
	return common;
      else
	{
	  float determ_factor = sqrtf (determ) / dir_dir;
	  float t0 = common - determ_factor;
	  float t1 = common + determ_factor;

//       std::cout << "    determ_factor = " << determ_factor << std::endl;
//       std::cout << "    t0       = " << t0 << std::endl;
//       std::cout << "    t1       = " << t1 << std::endl;

	  if (t0 > 0)
	    return t0;
	  else if (t1 > 0)
	    return t1;
	}
    }

//   std::cout << "    (giving up)" << std::endl;

  return 0;
}

void
Sphere::closest_intersect (Intersect &isec) const
{
  isec.set_obj_if_closer (this, intersection_distance (isec.ray));
//   std::cout << "isec.distance = " << isec.distance << std::endl;
}

void
Sphere::finish_intersect (Intersect &isec) const
{
  Space::dist_t dist = isec.distance;
  isec.point = isec.ray.extension (dist);
  isec.normal = (isec.point - center).unit ();
//   std::cout << "isec.point = " << isec.point << std::endl;
//   std::cout << "isec.normal = " << isec.normal << std::endl;
}

bool
Sphere::intersects (const Ray &ray) const
{
  Space::dist_t dist = intersection_distance (ray);
  return dist > 0 && dist < ray.len;
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
