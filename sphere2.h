// sphere2.h -- Alternative sphere surface
//
//  Copyright (C) 2007, 2008, 2009, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE2_H__
#define __SPHERE2_H__


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

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const;

  // Return a sampler for this surface, or zero if the surface doesn't
  // support sampling.  The caller is responsible for destroying
  // returned samplers.
  //
  virtual Sampler *make_sampler () const;

  // Sphere2 Sampler interface.
  //
  class Sampler : public Surface::Sampler
  {
  public:

    Sampler (const Sphere2 &_sphere) : sphere (_sphere) { }

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

    const Sphere2 &sphere;
  };

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Sphere2 &_sphere, const Vec &_onorm)
      : Surface::IsecInfo (ray), sphere (_sphere), onorm (_onorm)
    { }

    virtual Intersect make_intersect (const Media &media, RenderContext &context)
      const;

    const Sphere2 &sphere;

    // Intersection normal in SPHERE's local coordinate system.
    //
    Vec onorm;
  };

  // Return the texture coordinates for object-space position OPOS on
  // the sphere.
  //
  UV tex_coords (const Pos &opos) const
  {
    return UV (atan2 (opos.y, opos.x) * INV_PIf * 0.5f + 0.5f,
	       asin (clamp (opos.z, -1.f, 1.f)) * INV_PIf + 0.5f);
  }

};


}


#endif /* __SPHERE2_H__ */

// arch-tag: e633a2ec-7f36-4a52-89de-5ab76bdd934f
