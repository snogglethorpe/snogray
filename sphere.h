// sphere.h -- Sphere surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "surface.h"

namespace snogray {

class Sphere : public Surface
{
public:

  Sphere (const Material *mat, const Pos &_center, dist_t _radius)
    : Surface (mat), center (_center), radius (_radius)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return true; otherwise
  // return false.  ISEC_PARAMS maybe used to pass information to a later
  // call to Surface::intersect_info.
  //
  virtual bool intersect (Ray &ray, IsecParams &isec_params) const;

  // Return an Intersect object containing details of the intersection of
  // RAY with this surface; it is assumed that RAY does actually hit the
  // surface, and RAY's maximum bound (Ray::t1) gives the exact point of
  // intersection (the `intersect' method modifies RAY so that this is
  // true).  ISEC_PARAMS contains other surface-specific parameters
  // calculated by the previous call to Surface::intersects method.
  //
  virtual Intersect intersect_info (const Ray &ray,
				    const IsecParams &isec_params,
				    Trace &trace)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  Pos center;
  dist_t radius;
};

}

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
