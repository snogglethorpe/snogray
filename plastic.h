// plastic.h -- Plastic (thin, transmissive, reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PLASTIC_H__
#define __PLASTIC_H__

#include "material.h"
#include "medium.h"

namespace Snogray {

// "Plastic" is like our Glass material, but doesn't contain a medium, so
// all filtering effects take place during the surface transition.  This is
// a better match for typical "transparent" materials in many scene
// definition languages than Glass.
//
class Plastic : public Material
{
public:

  Plastic (const Color &col, float _ior = 1.5)
    : Material (1, Material::SHADOW_MEDIUM), color (col), ior (_ior)
  { }

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

  // The index of refraction here is only used for calculating surface
  // reflectance.
  //
  float ior;
};

}

#endif /* __PLASTIC_H__ */

// arch-tag: eb5d0a02-3751-421d-9ddd-5daf10d56a1e
