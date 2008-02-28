// sphere.h -- Sphere surface
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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

  Sphere (const Ref<const Material> &mat, const Pos &_center, dist_t _radius)
    : Surface (mat), center (_center), radius (_radius), axis (Vec (0, 0, 1))
  { }

  Sphere (const Ref<const Material> &mat,
	  const Pos &_center, const Vec &_radius)
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
    IsecInfo (const Ray &ray, const Sphere *_sphere)
      : Surface::IsecInfo (ray), sphere (_sphere)
    { }
    virtual Intersect make_intersect (Trace &trace) const;
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
