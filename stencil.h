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
    : opacity (_opacity), underlying_material (_underlying_material)
  {
    bump_map = _underlying_material->bump_map;
  }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;

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
