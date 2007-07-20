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
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with ISEC_CTX); otherwise return zero.
  //
  virtual IsecInfo *intersect (Ray &ray, IsecCtx &isec_ctx) const;

  // Return true if this surface blocks RAY coming from ISEC.  This should
  // be somewhat lighter-weight than Surface::intersect (and can handle
  // special cases for some surface types).
  //
  virtual bool shadows (const Ray &ray, const Intersect &isec) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Sphere *_sphere) : sphere (_sphere) { }
    virtual Intersect make_intersect (const Ray &ray, Trace &trace) const;
    const Sphere *sphere;
  };

  Pos center;
  dist_t radius;
};

}

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
