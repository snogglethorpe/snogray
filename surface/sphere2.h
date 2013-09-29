// sphere2.h -- Alternative sphere surface
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

#ifndef SNOGRAY_SPHERE2_H
#define SNOGRAY_SPHERE2_H

#include "geometry/spherical-coords.h"

#include "local-primitive.h"


namespace snogray {


// This is a sphere surface which transforms a canonical sphere instead of
// using a simple center position and radius like the Sphere class.
// Sphere2 objects use more memory than Sphere objects, but the transform
// allows more flexibility.
//
class Sphere2 : public LocalPrimitive
{
public:

  Sphere2 (const Ref<const Material> &mat, const Xform &local_to_world_xform)
    : LocalPrimitive (mat, local_to_world_xform)
  { }

  // If this surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::Renderable::IsecInfo object describing the intersection
  // (which should be allocated using placement-new with CONTEXT);
  // otherwise return zero.
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

private:

  class Sampler;
  class IsecInfo;

  // Return the texture coordinates for object-space position OPOS on
  // the sphere.
  //
  UV tex_coords (const Vec &opos) const
  {
    return z_axis_latlong (opos);
  }

};


}


#endif /* SNOGRAY_SPHERE2_H */

// arch-tag: e633a2ec-7f36-4a52-89de-5ab76bdd934f
