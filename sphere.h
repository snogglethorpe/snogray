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
    : Surface (mat), center (_center), radius (_radius), axis (Vec (0, 0, 1))
  { }

  Sphere (const Material *mat, const Pos &_center, const Vec &_radius)
    : Surface (mat), center (_center),
      radius (_radius.length ()), axis (_radius.unit ())
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with ISEC_CTX); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, const IsecCtx &isec_ctx) const;

  // Return the strongest type of shadowing effect this surface has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  virtual Material::ShadowType shadow (const ShadowRay &ray) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Sphere *_sphere) : sphere (_sphere) { }
    virtual Intersect make_intersect (const Ray &ray, Trace &trace) const;
    virtual const Surface *surface () const { return sphere; }
    const Sphere *sphere;
  };

  Pos center;
  dist_t radius;

  // A unit vector pointing along the "axis" of the sphere.
  //
  Vec axis;
};

}

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
