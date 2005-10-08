// intersect.h -- Datatype for recording object-ray intersection results
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __INTERSECT_H__
#define __INTERSECT_H__

#include "ray.h"
#include "color.h"
#include "obj.h"

namespace Snogray {

class Intersect
{
public:
  Intersect (const Ray &_ray) : ray (_ray), obj (0) { }

  bool hit () { !!obj; }

  void update (Obj *_obj)
  {
    dist_t dist = _obj->intersection_distance (ray);
    if (dist > 0 && (!obj || dist < distance))
      {
	obj = _obj;
	distance = dist;
      }
  }

  void finish ()
  {
    if (obj)
      {
	eye_dir = -ray.dir;
	point = ray.extension (distance);
	normal = obj->normal (point, eye_dir);
      }
  }

  // Ray which intersected something.
  Ray ray;

  // If non-zero, the object which RAY intersected, in which case DISTANCE
  // is valid; if zero, there is no known intersection.
  const Obj *obj;		// If 0, no intersection
  dist_t distance;		// Distance to intersection point on OBJ

  // Only valid once finish() is called, and if OBJ is non-NULL
  Pos point;
  Vec eye_dir;
  Vec normal;
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
