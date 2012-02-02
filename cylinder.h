// cylinder.h -- Cylindrical surface
//
//  Copyright (C) 2007-2012  Miles Bader <miles@gnu.org>
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

  // Cylinder Sampler interface.
  //
  class Sampler : public Surface::Sampler
  {
  public:

    Sampler (const Cylinder &_cylinder) : cylinder (_cylinder) { }

    // Return a sample of this surface.
    //
    virtual AreaSample sample (const UV &param) const;

    // Return a sample of this surface from VIEWPOINT, based on the
    // parameter PARAM.
    //
    virtual AngularSample sample_from_viewpoint (const Pos &viewpoint,
						 const UV &param)
      const;

    // If a ray from VIEWPOINT in direction DIR intersects this
    // surface, return an AngularSample as if the
    // Surface::Sampler::sample_from_viewpoint method had returned a
    // sample at the intersection position.  Otherwise, return an
    // AngularSample with a PDF of zero.
    //
    virtual AngularSample eval_from_viewpoint (const Pos &viewpoint,
					       const Vec &dir)
      const;

  private:

    const Cylinder &cylinder;
  };

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Cylinder &_cylinder, const Pos &_isec_point)
      : Surface::IsecInfo (ray), cylinder (_cylinder), isec_point (_isec_point)
    { }

    virtual Intersect make_intersect (const Media &media,
				      RenderContext &context)
      const;

    virtual TexCoords tex_coords () const;
    virtual Vec normal () const;

    const Cylinder &cylinder;

    // Intersection point in the cylinder's local coordinate system.
    //
    Pos isec_point;
  };

  // Return the texture coordinates for object-space position OPOS on
  // the cylinder.
  //
  UV tex_coords (const Pos &opos) const
  {
    return UV (float (atan2 (opos.y, opos.x)) * INV_PIf * 0.5f + 0.5f,
	       float (opos.z) * 0.5f + 0.5f);
  }

};


}

#endif // SNOGRAY_CYLINDER_H


// arch-tag: 583e4c68-5f8f-4d18-9100-3abab4b525ce
