// stencil.h -- Masking material for partial transparency/translucency
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __STENCIL_H__
#define __STENCIL_H__

#include "tex.h"
#include "material.h"


namespace snogray {


// A material that makes parts of an underlying material transparent
// or translucent.
//
class Stencil : public Material
{
public:

  Stencil (const TexVal<Color> &_opacity,
	   const Ref<const Material> &_underlying_material)
    : Material (PARTIALLY_OCCLUDING),
      opacity (_opacity), underlying_material (_underlying_material)
  {
    bump_map = _underlying_material->bump_map;
  }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;

  // Return the transmittance of this material at the intersection
  // described by ISEC_INFO in medium MEDIUM.
  //
  // Note that this method only applies to "simple"
  // transparency/translucency, where transmitted rays don't change
  // direction; materials that are conceptually "transparent," but
  // which exhibit more complex effects like refraction (which change
  // the direction) may return zero from this method.
  //
  virtual Color transmittance (const Surface::IsecInfo &isec_info,
			       const Medium &medium)
    const;

  // Opacity of material.
  //
  TexVal<Color> opacity;

  // Material underlying the stencil, which handles any
  // non-transparent areas.
  //
  Ref<const Material> underlying_material;
};


}

#endif // __STENCIL_H__
