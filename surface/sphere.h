// sphere.h -- Sphere surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SPHERE_H
#define SNOGRAY_SPHERE_H

#include "geometry/spherical-coords.h"

#include "primitive.h"


namespace snogray {


class Sphere : public Primitive
{
public:

  Sphere (const Ref<const Material> &mat, dist_t _radius, const Frame &_frame)
    : Primitive (mat), radius (_radius), frame (_frame)
  { }

  Sphere (const Ref<const Material> &mat, const Pos &_center, dist_t _radius)
    : Primitive (mat), radius (_radius), frame (_center)
  { }

  Sphere (const Ref<const Material> &mat,
	  const Pos &_center, const Vec &_radius)
    : Primitive (mat),
      radius (_radius.length ()), frame (_center, _radius.unit ())
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

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Transform the geometry of this surface by XFORM.
  //
  virtual void transform (const Xform &xform);

  // Add light-samplers for this surface in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const TexVal<Color> &intensity,
		 std::vector<const Light::Sampler *> &samplers)
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

  dist_t radius;

  // A frame describing the sphere's coordinate system.
  //
  Frame frame;
};


}

#endif // SNOGRAY_SPHERE_H


// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
