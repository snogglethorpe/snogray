// sphere.h -- Sphere surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Sphere *_sphere)
      : Surface::IsecInfo (ray), sphere (_sphere)
    { }
    virtual Intersect make_intersect (const Media &media, RenderContext &context)
      const;
    const Sphere *sphere;
  };

  dist_t radius;

  // A frame describing the sphere's coordinate system.
  //
  Frame frame;
};


}

#endif // __SPHERE_H__


// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
