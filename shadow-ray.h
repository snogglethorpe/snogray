// shadow-ray.h -- Special type of Ray used for shadowing calculations
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SHADOW_RAY_H__
#define __SHADOW_RAY_H__

#include "ray.h"
#include "intersect.h"


namespace snogray {


class Light;


// A shadow ray is a subclass of Ray used in shadowing calculations; it
// carries some additional miscellaneous info which can sometimes be
// useful.
//
class ShadowRay : public Ray
{
public:

  // Make a shadow ray.  Note that the light is optional,
  //
  ShadowRay (const Intersect &_isec, const Vec &light_dir,
	     dist_t min_dist, dist_t max_dist,
	     const Light *_light = 0)
    : Ray (_isec.pos, light_dir, min_dist, max_dist),
      isec (_isec), light (_light)
  { }

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

  // Light to which the shadow-ray is associated.  This may be zero.
  //
  const Light *light;
};   


}

#endif // __SHADOW_RAY_H__


// arch-tag: f0c64e72-dacd-4ebc-b75e-2714c9c7c0f8
