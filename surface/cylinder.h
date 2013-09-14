// cylinder.h -- Cylindrical surface
//
//  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_CYLINDER_H
#define SNOGRAY_CYLINDER_H

#include "local-primitive.h"


namespace snogray {


class Cylinder : public LocalPrimitive
{
public:

  Cylinder (const Ref<const Material> &mat, const Xform &local_to_world_xform)
    : LocalPrimitive (mat, local_to_world_xform)
  { }

  // Constructor for making a cylinder from a base/axis/radius
  //
  Cylinder (const Ref<const Material> &mat,
	    const Pos &base, const Vec &axis, float radius)
    : LocalPrimitive (mat, xform (base, axis, radius))
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const;

  // Return true if this surface completely occludes RAY.  If it does
  // not completely occlude RAY, then return false, and multiply
  // TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
  // MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it).
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const;

  // Return a sampler for this surface, or zero if the surface doesn't
  // support sampling.  The caller is responsible for destroying
  // returned samplers.
  //
  virtual Sampler *make_sampler () const;

  // Return a transformation that will transform a canonical cylinder to a
  // cylinder with the given base/axis/radius.
  //
  static Xform xform (const Pos &base, const Vec &axis, float radius);

private:

  class Sampler;
  class IsecInfo;

  // Return 2d texture coordinates for object-space position OPOS on
  // the cylinder.
  //
  UV tex_coords_uv (const Pos &opos) const
  {
    return UV (float (atan2 (opos.y, opos.x)) * INV_PIf * 0.5f + 0.5f,
	       float (opos.z) * 0.5f + 0.5f);
  }

  // Returns a vector along a radius of the cylinder pointing at a
  // location on the circumference corresponding to PARAM.
  //
  Vec radius (const UV &param) const
  {
    float theta = param.u * 2 * PIf;
    return Vec (cos (theta), sin (theta), 0);
  }
};


}

#endif // SNOGRAY_CYLINDER_H


// arch-tag: 583e4c68-5f8f-4d18-9100-3abab4b525ce
