// cook-torrance.h -- Cook-Torrance material
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COOK_TORRANCE_H__
#define __COOK_TORRANCE_H__

#include "tex.h"
#include "material.h"
#include "fresnel.h"

namespace snogray {


class CookTorrance : public Material
{
public:

  CookTorrance (const TexVal<Color> &col, const TexVal<Color> &gloss_col,
		const TexVal<float> &_m, const Ior &_ior = 1.5)
    : color (col), gloss_color (gloss_col), m (_m), ior (_ior)
  { }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;

  TexVal<Color> color, gloss_color;

  // Cook Torrance parameters:

  // m:  RMS slope of microfacets -- large m means more spread out
  // reflections.
  //
  TexVal<float> m;

  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;
};


}

#endif // __COOK_TORRANCE_H__


// arch-tag: 73c818bb-1305-412f-a616-6950b8d9ef39
