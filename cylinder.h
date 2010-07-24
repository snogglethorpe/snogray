// cylinder.h -- Cylindrical surface
//
//  Copyright (C) 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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
  virtual bool intersects (const ShadowRay &ray, RenderContext &context) const;

  // Return a transformation that will transform a canonical cylinder to a
  // cylinder with the given base/axis/radius.
  //
  static Xform xform (const Pos &base, const Vec &axis, float radius);

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Cylinder *_cylinder, const Pos &_isec_point)
      : Surface::IsecInfo (ray), cylinder (_cylinder), isec_point (_isec_point)
    { }

    virtual Intersect make_intersect (const Media &media,
				      RenderContext &context)
      const;

    const Cylinder *cylinder;

    // Intersection point in the cylinder's local coordinate system.
    //
    Pos isec_point;
  };

};


}

#endif // __CYLINDER_H__


// arch-tag: 583e4c68-5f8f-4d18-9100-3abab4b525ce
