// material.h -- Surface material datatype
//
//  Copyright (C) 2005-2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MATERIAL_H
#define SNOGRAY_MATERIAL_H

#include <vector>

#include "color/color.h"
#include "util/ref.h"
#include "texture/tex.h"
#include "surface/surface-renderable.h"


namespace snogray {

class Light;
class Intersect;
class Medium;
class Bsdf;
class Primitive;


class Material : public RefCounted
{
public:

  // Flags for materials.  "Typical" materials usually have no flags set.
  //
  enum {
    // This material may return a value other than zero from the
    // Material::transmittance method (see the method comment for more
    // detail).
    //
    PARTIALLY_OCCLUDING = 0x1,

    // This material may emit light.
    //
    EMITS_LIGHT = 0x2,

    // This material may use texture-coordinates in its
    // occlusion/transparency calculations, so the appropriate version
    // of Material::occludes or Material::transmittance with
    // texture-coordinates as an argument should be called.
    //
    OCCLUSION_REQUIRES_TEX_COORDS = 0x4
  };

  Material (unsigned _flags = 0) : bump_map (0), flags (_flags)  { }
  virtual ~Material () { }

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
  virtual Bsdf *get_bsdf (const Intersect &/*isec*/,
			  const TexCoords &/*tex_coords*/)
    const
  { return 0; }

  // Return the medium of this material (used only for refraction).
  //
  virtual const Medium *medium () const { return 0; }

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color Le (const Intersect &/*isec*/, const TexCoords &/*tex_coords*/)
    const
  { return 0; }

  // Return true if this material emits light.
  //
  bool emits_light () const { return (flags & EMITS_LIGHT); }

  // Return true if Material::transmittance will always return zero.
  //
  bool fully_occluding () const { return ! (flags & PARTIALLY_OCCLUDING); }

  // Return true if occlusion-testing for this material requires
  // texture-coordinates, meaning that the variant of
  // Material::occludes which takes a TexCoords argument should be
  // called (and internally, that the version of
  // Material::transmittance taking a TexCoords argument should be
  // called).
  //
  bool occlusion_requires_tex_coords () const
  {
    return (flags & OCCLUSION_REQUIRES_TEX_COORDS);
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
  // There are two variants of this method: one which takes texture-
  // coordinates as an argument, and one which does not.  If the
  // material flag Material::OCCLUSION_REQUIRES_TEX_COORDS is set, the
  // former will be called, otherwise the latter.  This is done
  // because calculating texture-coordinate can be expensive, and we'd
  // like to only do those calculations if necessary.  Subclasses
  // overriding this method should set the flag appropriately and
  // override whichever variants of Material::transmittance may be
  // called as a result.
  //
  virtual Color transmittance (
		  const Surface::Renderable::IsecInfo &/*isec_info*/,
		  const Medium &/* medium */)
    const
  {
    return 0;
  }
  virtual Color transmittance (
		  const Surface::Renderable::IsecInfo &/*isec_info*/,
		  const TexCoords &/*tex_coords*/,
		  const Medium &/* medium */)
    const
  {
    return 0;
  }

  // Return true if this material completely occludes a ray at the
  // intersection described by ISEC_INFO, with texture-coordinates
  // TEX_COORDS.  If it does not, then return false, and multiply
  // TOTAL_TRANSMITTANCE by the transmittance of the material at
  // ISEC_INFO in medium MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it).
  //
  // There are two variants of this method: one which takes
  // texture-coordinates as an argument, and one which does not.  If
  // the material flag Material::OCCLUSION_REQUIRES_TEX_COORDS is set,
  // the former should be called, otherwise the latter.  This is done
  // because calculating texture-coordinate can be expensive, and we'd
  // like to only do those calculations if necessary.
  //
  bool occludes (const Surface::Renderable::IsecInfo &isec_info,
		 const Medium &medium,
		 Color &total_transmittance)
    const
  {
    // avoid calling Material::transmittance if possible
    if (fully_occluding ())
      return true;

    total_transmittance *= transmittance (isec_info, medium);
    return total_transmittance < Eps;
  }
  bool occludes (const Surface::Renderable::IsecInfo &isec_info,
		 const TexCoords &tex_coords,
		 const Medium &medium,
		 Color &total_transmittance)
    const
  {
    // avoid calling Material::transmittance if possible
    if (fully_occluding ())
      return true;

    total_transmittance *= transmittance (isec_info, tex_coords, medium);
    return total_transmittance < Eps;
  }

  // If this is a light-emitting material, call PRIMITIVE's
  // Primitive::add_light method with an appropriate intensity to add
  // a Light::Sampler to SAMPLERS (for non-light-emitting materials,
  // do nothing).
  //
  virtual void add_light_samplers (
		 const Primitive &/*primitive*/,
		 std::vector<const Light::Sampler *> &/*samplers*/)
    const
  { }

  Ref<const Tex<float> > bump_map;

  unsigned char flags;
};


}

#endif /* SNOGRAY_MATERIAL_H */


// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
