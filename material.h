// material.h -- Surface material datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "color.h"
#include "brdf.h"
#include "lambert.h"
#include "ray.h"

namespace snogray {

class Light;
class Surface;
class Intersect;
class Trace;

class Material
{
public:

  // Types of shadow, from none to completely opaque.  Greater values
  // are "stronger" (block more).
  //
  enum ShadowType { SHADOW_NONE, SHADOW_MEDIUM, SHADOW_OPAQUE };

  Material (const Color &col, const Brdf *brdf = lambert,
	    ShadowType _shadow_type = SHADOW_OPAQUE)
    : color (col), brdf (* (brdf ? brdf : lambert)), light (0),
      shadow_type (_shadow_type)
  { }
  Material (const Color &col, ShadowType _shadow_type)
    : color (col), brdf (*lambert), shadow_type (_shadow_type)
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

  Color color;
  const Brdf &brdf;

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
