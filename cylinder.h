// cylinder.h -- Cylindrical surface
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CYLINDER_H__
#define __CYLINDER_H__


#include "local-surface.h"


namespace snogray {


class Cylinder : public LocalSurface
{
public:

  Cylinder (const Material *mat, const Xform &local_to_world_xform)
    : LocalSurface (mat, local_to_world_xform)
  { }

  // Constructor for making a cylinder from a base/axis/radius
  //
  Cylinder (const Material *mat, const Pos &base, const Vec &axis, float radius)
    : LocalSurface (mat, xform (base, axis, radius))
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

  // Return a transformation that will transform a canonical cylinder to a
  // cylinder with the given base/axis/radius.
  //
  static Xform xform (const Pos &base, const Vec &axis, float radius);

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Cylinder *_cylinder, coord_t ix, coord_t iy)
      : Surface::IsecInfo (ray), cylinder (_cylinder), isec_x (ix), isec_y (iy)
    { }

    virtual Intersect make_intersect (Trace &trace) const;

    virtual const Surface *surface () const { return cylinder; }

    const Cylinder *cylinder;

    // X/Y coordinates in the cylinder's local coordinate system of the
    // intersection; used to calculate the normal.
    //
    coord_t isec_x, isec_y;
  };

};


}


#endif /* __CYLINDER_H__ */

// arch-tag: 583e4c68-5f8f-4d18-9100-3abab4b525ce
