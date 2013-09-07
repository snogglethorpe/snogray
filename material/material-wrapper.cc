// material-wrapper.cc -- Helper class for Material wrappers
//
//  Copyright (C) 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "material-wrapper.h"


using namespace snogray;


// Create a wrapper around MATERIAL.
//
MaterialWrapper::MaterialWrapper (const Ref<const Material> &_material)
  : Material (_material->flags), material (_material)
{
}


// Return a new Bsdf object for this material instantiated at ISEC,
// with texture-coordinates TEX_COORDS.
//
// Bsdf objects are allocated extremely often, and they need to
// follow certain rules:
//
//  (1) They should be allocated using ISEC as an arena, i.e., with
//      "new (isec) SomeBsdfClass (...)"
//	  This allocator is very fast, and does no locking.
//
//  (2) They should not depend on their destructor being called,
//      because it won't be.  The memory they occupy (allocated
//      using ISEC) will be eventually freed in bulk.
//
// Consequently, it's unnecessary to do careful tracking of Bsdf
// objects for memory management purposes, and no attempt is made to
// do so.
//
Bsdf *MaterialWrapper::get_bsdf (const Intersect &isec,
				 const TexCoords &tex_coords)
  const
{
  return material->get_bsdf (isec, tex_coords);
}

// Return the medium of this material (used only for refraction).
//
const Medium *
MaterialWrapper::medium () const
{
  return material->medium ();
}

// Return emitted radiance from this light, at the point described by ISEC.
//
Color
MaterialWrapper::Le (const Intersect &isec, const TexCoords &tex_coords)
  const
{
  return material->Le (isec, tex_coords);
}

// Return the transmittance of this material at the intersection
// described by ISEC_INFO, with texture-coordinates TEX_COORDS, in
// medium MEDIUM.
//
// Note that this method only applies to "simple"
// transparency/translucency, where transmitted rays don't change
// direction; materials that are conceptually "transparent," but
// which exhibit more complex effects like refraction (which change
// the direction) may return zero from this method.
//
Color
MaterialWrapper::transmittance (const Surface::IsecInfo &isec_info,
				const Medium &medium)
  const
{
  return material->transmittance (isec_info, medium);
}
Color
MaterialWrapper::transmittance (const Surface::IsecInfo &isec_info,
				const TexCoords &tex_coords,
				const Medium &medium)
  const
{
  return material->transmittance (isec_info, tex_coords, medium);
}

// If this is a light-emitting material, call PRIMITIVE's
// Primitive::add_light method with an appropriate intensity to add a
// light to LIGHTS (for non-light-emitting materials, do nothing).
//
void
MaterialWrapper::add_light (const Primitive &primitive,
			    std::vector<Light *> &lights)
  const
{
  material->add_light (primitive, lights);
}
