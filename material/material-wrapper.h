// material-wrapper.h -- Helper class for Material wrappers
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

#ifndef SNOGRAY_MATERIAL_WRAPPER_H
#define SNOGRAY_MATERIAL_WRAPPER_H

#include "texture/tex.h"

#include "material.h"


namespace snogray {


// This is a helper class for subclasses of material that wrap another
// material and delegate most operations to the wrapped material.
//
// It simply duplicates the wrapped material's flags, and passes
// through all virtual calls.  Classes that use MaterialWrapper should
// subclass it instead of subclassing Material, and override any
// methods that need to be handled individually (and if some flags
// need to be treated specially, do that in the constructor).
//
class MaterialWrapper : public Material
{
public:

  // Create a wrapper around MATERIAL.
  //
  MaterialWrapper (const Ref<const Material> &_material);


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
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

  // Return the medium of this material (used only for refraction).
  //
  virtual const Medium *medium () const;

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color Le (const Intersect &isec, const TexCoords &tex_coords)
    const;

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
  virtual Color transmittance (const Surface::Renderable::IsecInfo &isec_info,
			       const Medium &medium)
    const;
  virtual Color transmittance (const Surface::Renderable::IsecInfo &isec_info,
			       const TexCoords &tex_coords,
			       const Medium &medium)
    const;

  // If this is a light-emitting material, call PRIMITIVE's
  // Primitive::add_light method with an appropriate intensity to add
  // a Light::Sampler to SAMPLERS (for non-light-emitting materials,
  // do nothing).
  //
  virtual void add_light_samplers (
		 const Primitive &primitive,
		 std::vector<const Light::Sampler *> &samplers)
    const;


protected:

  // Material which is wrapped.
  //
  Ref<const Material> material;
};


}

#endif // SNOGRAY_MATERIAL_WRAPPER_H
