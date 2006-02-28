// material.h -- Surface material datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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

namespace Snogray {

class Ray;
class Light;
class Surface;
class Intersect;
class Trace;

class Material
{
public:

  enum ShadowType { SHADOW_OPAQUE, SHADOW_NONE, SHADOW_MEDIUM };

  Material (const Color &col, const Brdf *brdf = lambert)
    : color (col), brdf (* (brdf ? brdf : lambert))
  { }
  virtual ~Material ();

  virtual Color render (const Intersect &isec) const;

  // The general sort of shadow this material will cast.  This value
  // should never change for a given material, so can be cached.
  //
  virtual ShadowType shadow_type () const;

  // Calculate the shadowing effect of SURFACE on LIGHT_RAY (which points at
  // the light, not at the surface).  The "non-shadowed" light has color
  // LIGHT_COLOR; it's also this method's job to find any further
  // shadowing surfaces.
  //
  virtual Color shadow (const Surface *surface,
			const Ray &light_ray, const Color &light_color,
			const Light &light, Trace &trace)
    const;

  Color color;
  const Brdf &brdf;
};

}

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
