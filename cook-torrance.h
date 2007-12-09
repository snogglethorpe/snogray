// cook-torrance.h -- Cook-Torrance material
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COOK_TORRANCE_H__
#define __COOK_TORRANCE_H__

#include "material.h"
#include "fresnel.h"

namespace snogray {


class CookTorrance : public Material
{
public:

  CookTorrance (const Color &col, const Color &spec_col,
		float _m, const Ior &_ior = 1.5)
    : color (col), specular_color (spec_col), m (_m), ior (_ior)
  { }

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &isec) const;

  Color color, specular_color;

  // Cook Torrance parameters:

  // m:  RMS slope of microfacets -- large m means more spread out
  // reflections.
  //
  float m;

  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;
};


}

#endif /* __COOK_TORRANCE_H__ */


// arch-tag: 73c818bb-1305-412f-a616-6950b8d9ef39
