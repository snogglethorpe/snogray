// glow.cc -- Constant-color reflectance function
//
//  Copyright (C) 2005-2008, 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface/primitive.h"

#include "glow.h"


using namespace snogray;


Glow::Glow (const TexVal<Color> &col)
  : Material (EMITS_LIGHT),
    color (col)
{ }

Glow::Glow (const TexVal<Color> &col,
	    const Ref<const Material> &_underlying_material)
  : Material (EMITS_LIGHT),
    color (col), underlying_material (_underlying_material)
{
  bump_map = _underlying_material->bump_map;
}

// Return emitted radiance from this light, at the point described by ISEC.
//
Color
Glow::Le (const Intersect &isec, const TexCoords &tex_coords) const
{
  return isec.back ? 0 : color.eval (tex_coords);
}

// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
Glow::get_bsdf (const Intersect &isec, const TexCoords &tex_coords) const
{
  return (underlying_material
	  ? underlying_material->get_bsdf (isec, tex_coords)
	  : 0);
}

// Return the medium of this material (used only for refraction).
//
const Medium *
Glow::medium () const
{
  return underlying_material ? underlying_material->medium () : 0;
} 

// If this is a light-emitting material, call PRIMITIVE's
// Primitive::add_light method with an appropriate intensity to add a
// Light::Sampler to SAMPLERS (for non-light-emitting materials, do
// nothing).
//
void
Glow::add_light_samplers (const Primitive &primitive,
			  std::vector<const Light::Sampler *> &samplers)
  const
{
  primitive.add_light_samplers (color, samplers);
}


// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
