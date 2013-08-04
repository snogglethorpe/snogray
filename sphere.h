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

#include "spherical-coords.h"
#include "surface-sampler.h"

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

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add a single area light, using this surface's shape, to LIGHTS,
  // with with intensity INTENSITY.  An error will be signaled if this
  // surface does not support lighting.
  //
  virtual void add_light (const TexVal<Color> &intensity,
			  std::vector<Light *> &lights)
    const;

  // Return a sampler for this surface, or zero if the surface doesn't
  // support sampling.  The caller is responsible for destroying
  // returned samplers.
  //
  virtual Sampler *make_sampler () const;

  // Sphere Sampler interface.
  //
  class Sampler : public Surface::Sampler
  {
  public:

    Sampler (const Sphere &_sphere) : sphere (_sphere) { }

    // Return a sample of this surface.
    //
    virtual AreaSample sample (const UV &param) const;

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

    const Sphere &sphere;
  };

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Sphere &_sphere)
      : Surface::IsecInfo (ray), sphere (_sphere)
    { }
    virtual Intersect make_intersect (const Media &media, RenderContext &context)
      const;
    virtual TexCoords tex_coords () const;
    virtual Vec normal () const;
    const Sphere &sphere;
  };

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
