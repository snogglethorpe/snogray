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

  // If DIST is closer than the current intersection distance, update the
  // current intersection object and distance to be _OBJ and DIST,
  // otherwise do nothing.
  void set_obj_if_closer (const Obj *_obj, Vec::dist_t dist)
  {
    if (dist > 0 && (!obj || dist < distance))
      {
	obj = _obj;
	distance = dist;
      }
  }

  void finish () { if (obj) obj->finish_intersect (*this); }

  // Return the color seen from RAY's point of view, of the intersected
  // object; if there is no current intersected object, black is returned.
  Color render (const Vec &eye_dir, const Vec &light_dir,
		const Color &light_color)
    const;

  // Ray which intersected something.
  Ray ray;

  // If non-zero, the object which RAY intersected, in which case DISTANCE
  // is valid; if zero, there is no known intersection.
  const Obj *obj;		// If 0, no intersection
  Vec::dist_t distance;		// Distance to intersection point on OBJ

  // The following are only valid once finish() has been called, and are
  // only valid if OBJ is non-zero.
  Pos point;			// The actual point of intersection
  Vec normal;			// OBJ's surface normal at POINT
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
