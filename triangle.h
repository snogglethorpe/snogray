// triangle.h -- Triangle object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "primary-obj.h"

namespace Snogray {

class Triangle : public PrimaryObj
{
public:

  Triangle (const Material *mat, const Pos &_v0, const Pos &_v1, const Pos &_v2)
    : PrimaryObj (mat), v0 (_v0), v1 (_v1), v2 (_v2)
  { }

  virtual dist_t intersection_distance (const Ray &ray) const;

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

  Pos v0, v1, v2;
};

}

#endif /* __TRIANGLE_H__ */

// arch-tag: cf7f49db-937d-4328-b7c0-3959241a191e
