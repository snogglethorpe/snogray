// glow.cc -- Constant-color reflectance function
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "glow.h"
#include "scene.h"		// for inlined Trace::shadow method

using namespace snogray;


Glow::Glow (const TexVal<Color> &col)
  : Material (Material::SHADOW_OPAQUE, true), color (col)
{ }

Glow::Glow (const TexVal<Color> &col,
	    const Ref<const Material> &_underlying_material)
  : Material (_underlying_material->shadow_type, true),
    color (col), underlying_material (_underlying_material)
{
  bump_map = _underlying_material->bump_map;
}

// Return emitted radiance from this light, at the point described by ISEC.
//
Color
Glow::le (const Intersect &isec) const
{
  return isec.back ? 0 : color.eval (isec);
}

// Return a new BRDF object for this material instantiated at ISEC.
//
Brdf *
Glow::get_brdf (const Intersect &isec) const
{
  return underlying_material ? underlying_material->get_brdf (isec) : 0;
}

// Return the medium of this material (used only for refraction).
//
const Medium *
Glow::medium () const
{
  return underlying_material ? underlying_material->medium () : 0;
} 


// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
