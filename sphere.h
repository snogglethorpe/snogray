// sphere.h -- Sphere object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "primary-obj.h"

namespace Snogray {

class Sphere : public PrimaryObj
{
public:

  Sphere (const Material *mat, const Pos &_center, dist_t _radius)
    : PrimaryObj (mat), center (_center), radius (_radius)
  { }

  // Return the distance from RAY's origin of the closest intersection of
  // this object with RAY, or 0 if there is none.
  //
  virtual dist_t intersection_distance (const Ray &ray) const;

  // Given that RAY's origin is known to lie on this object, return the
  // distance from RAY's origin to the _next_ closest intersection of this
  // object with RAY, or 0 if there is none.
  //
  virtual dist_t next_intersection_distance (const Ray &ray) const;

  // Returns the normal vector for this surface at POINT.
  // INCOMING is the direction of the incoming ray that has hit POINT;
  // this can be used by dual-sided objects to decide which side's
  // normal to return.
  //
  virtual Vec normal (const Pos &point, const Vec &incoming) const;

  // Return a bounding box for this object.
  //
  virtual BBox bbox () const;

private:

  dist_t intersection_distance (const Ray &ray, bool closest_ok) const;

  Pos center;
  dist_t radius;
};

}

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
