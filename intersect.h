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

  Intersect (const Ray &_ray, dist_t horizon)
    : ray (_ray), obj (0)
  {
    ray.set_len (horizon);
  }

  void update (const Obj *_obj)
  {
    dist_t dist = _obj->intersection_distance (ray);
    if (dist > 0 && (!obj || dist < ray.len))
      {
	obj = _obj;
	ray.set_len (dist);
      }
  }

  void finish ()
  {
    if (obj)
      {
	point = ray.end ();
	normal = obj->normal (point, ray.dir);
	reverse = normal.dot (ray.dir) > 0;
      }
  }

  // Ray which intersected something.
  Ray ray;

  // If non-zero, the object which RAY intersected; if zero, there is no
  // known intersection.
  //
  const Obj *obj;		// If 0, no intersection

  // Only valid once finish() is called, and if OBJ is non-NULL
  //
  Pos point;			// Point where RAY intersects OBJ
  Vec normal;			// Surface normal at POINT
  bool reverse;			// True if NORMAL points away from RAY
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
