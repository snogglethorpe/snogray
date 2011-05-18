// sphere.h -- Sphere surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011  Miles Bader <miles@gnu.org>
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
    const Sphere &sphere;
  };

  // Return the texture coordinates for object-space position OPOS on
  // the sphere.
  //
  UV tex_coords (const Vec &opos) const
  {
    return UV (atan2 (opos.y, opos.x) * INV_PIf * 0.5f + 0.5f,
	       asin (clamp (opos.z / radius, -1.f, 1.f)) * INV_PIf + 0.5f);
  }

  dist_t radius;

  // A frame describing the sphere's coordinate system.
  //
  Frame frame;
};


}

#endif // __SPHERE_H__


// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
