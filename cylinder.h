// cylinder.h -- Cylindrical surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
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

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Cylinder *_cylinder, coord_t ix, coord_t iy)
      : cylinder (_cylinder), isec_x (ix), isec_y (iy)
    { }

    virtual Intersect make_intersect (const Ray &ray, Trace &trace) const;

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
