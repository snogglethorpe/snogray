// shadow-ray.h -- Special type of Ray used for shadowing calculations
//
//  Copyright (C) 2005, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SHADOW_RAY_H__
#define __SHADOW_RAY_H__

#include "xform-base.h"
#include "intersect.h"

#include "ray.h"


namespace snogray {


// A shadow ray is a subclass of Ray used in shadowing calculations; it
// carries some additional miscellaneous info which can sometimes be
// useful.
//
class ShadowRay : public Ray
{
public:

  // Make a shadow ray.
  //
  ShadowRay (const Intersect &_isec, const Vec &light_dir,
	     dist_t min_dist, dist_t max_dist)
    : Ray (_isec.normal_frame.origin, light_dir, min_dist, max_dist), isec (_isec)
  { }

  // Ray-to-shadow-ray conversion.
  //
  ShadowRay (const Ray &ray, const Intersect &_isec)
    : Ray (ray), isec (_isec)
  { }

  // Ray transformation.
  //
  ShadowRay operator* (const XformBase<dist_t> &xform) const
  {
    return ShadowRay (as_ray() * xform, isec);
  }

  // Return a reference to our Ray base-class.  This is useful for
  // applying transforms where only the Ray result is desired --
  // transforming only the Ray part is more efficient than transforming
  // the whole ShadowRay and throwing away the unneeded parts of the
  // result.
  //
  Ray &as_ray () { return *this; }
  const Ray &as_ray () const { return *this; }

  // Intersection from which the shadow-ray came.
  //
  const Intersect &isec;
};   


}

#endif // __SHADOW_RAY_H__


// arch-tag: f0c64e72-dacd-4ebc-b75e-2714c9c7c0f8
