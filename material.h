// material.h -- Surface material datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "color.h"
#include "ray.h"


namespace snogray {

class Light;
class Intersect;
class Brdf;


class Material
{
public:

  // Types of shadow, from none to completely opaque.  Greater values
  // are "stronger" (block more).
  //
  enum ShadowType { SHADOW_NONE, SHADOW_MEDIUM, SHADOW_OPAQUE };

  Material (ShadowType _shadow_type = SHADOW_OPAQUE)
    : light (0), shadow_type (_shadow_type)
  { }
  virtual ~Material () { }

  virtual Color render (const Intersect &isec) const;

  // Shadow LIGHT_RAY, which points to a light with (apparent) color
  // LIGHT_COLOR. and return the shadow color.  This is basically like
  // the `render' method, but calls the material's `shadow' method
  // instead of its `render' method.
  //
  // Note that this method is only used for `non-opaque' shadows --
  // opaque shadows (the most common kind) don't use it!
  //
  virtual Color shadow (const Intersect &isec, const Ray &light_ray,
			const Color &light_color, const Light &light)
    const;

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &/*isec*/) const { return 0; }

  // If this material is bound to a light, the light, otherwise zero.
  //
  const Light *light;

  // The general sort of shadow this material will cast.  This value
  // should never change for a given material, so can be cached.
  //
  const ShadowType shadow_type;
};


}

#endif /* __MATERIAL_H__ */


// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
