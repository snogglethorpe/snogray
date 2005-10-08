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

  virtual dist_t intersection_distance (const Ray &ray) const;

  // Returns the normal vector for this surface at POINT.
  // EYE_DIR points to the direction the objects is being viewed from;
  // this can be used by dual-sided objects to decide which side's
  // normal to return.
  //
  virtual Vec normal (const Pos &point, const Vec &eye_dir) const;

  // Return a bounding box for this object.
  //
  virtual BBox bbox () const;

private:

  Pos center;
  dist_t radius;
};

}

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
