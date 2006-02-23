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

namespace Snogray {

class Surface;

// This just packages up the result of a scene intersection search and
// some handy values calculated from it.  It is passed to rendering
// methods.
//
class Intersect
{
public:

  Intersect (const Ray &_ray, const Surface *_surface,
	     const Pos _point, const Vec _normal, bool _back,
	     const void *_smoothing_group = 0)
    : ray (_ray), surface (_surface),
      point (_point), normal (_back ? -_normal : _normal), back (_back),
      smoothing_group (_smoothing_group)
  { }

  // For surfaces with non-interpolated normals, we can calculate
  // whether it's a backface or not using the normal; they typically
  // also have a zero smoothing group, so we omit that parameter.
  //
  Intersect (const Ray &_ray, const Surface *_surface,
	     const Pos _point, const Vec _normal)
    : ray (_ray), surface (_surface),
      point (_point), normal (_normal), back (dot (normal, _ray.dir) > 0),
      smoothing_group (0)
  {
    // We want to flip the sign on `normal' if `back' is true, but we've
    // declared `normal' const to avoid anybody mucking with it...
    //
    if (back)
      const_cast<Vec&> (normal) = -normal;
  }

  // Ray which intersected something; its endpoint is the point of intersection.
  const Ray ray;

  // The surface which RAY intersected.  This should always be non-zero
  // (it's not a reference because all uses are as a pointer).
  //
  const Surface *surface;

  // Details of the intersection.
  //
  const Pos point;		// Point where RAY intersects SURFACE
  const Vec normal;		// Surface normal at POINT
  const bool back;		// True if RAY hit the back of SURFACE

  // Cached value of SURFACE's smoothing group.
  //
  const void *smoothing_group;
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
