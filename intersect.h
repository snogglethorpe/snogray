// intersect.h -- Datatype for recording scene-ray intersection result
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

// This just packages up the result of a scene intersection search and
// some handy values calculated from it.  It is passed to rendering
// methods.
//
class Intersect
{
public:

  Intersect (const Ray &_ray, const Obj *_obj)
    : ray (_ray), obj (_obj),
      point (_ray.end()),
      normal (_obj->normal (point, _ray.dir)),
      reverse (normal.dot (_ray.dir) > Eps)
  {
    // We want to flip the sign on `normal' if `reverse' is true, but we've
    // declared `normal' const to avoid anybody mucking with it...
    //
    if (reverse)
      const_cast<Vec&> (normal) = -normal;
  }

  // Ray which intersected something; its endpoint is the point of intersection.
  const Ray ray;

  // The object which RAY intersected.  This should always be non-zero
  // (it's not a reference because all uses are as a pointer).
  //
  const Obj *obj;

  // Details of the intersection.
  //
  const Pos point;		// Point where RAY intersects OBJ
  const Vec normal;		// Surface normal at POINT
  const bool reverse;		// True if NORMAL points away from RAY
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
