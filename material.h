// material.h -- Surface material datatype
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

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <vector>

#include "color.h"
#include "ref.h"
#include "tex.h"


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
    EMITS_LIGHT = 0x2
  };

  Material (unsigned _flags = 0) : bump_map (0), flags (_flags)  { }
  virtual ~Material () { }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &/*isec*/) const { return 0; }

  // Return the medium of this material (used only for refraction).
  //
  virtual const Medium *medium () const { return 0; }

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color Le (const Intersect &/*isec*/) const { return 0; }

  // Return true if this material emits light.
  //
  bool emits_light () const { return (flags & EMITS_LIGHT); }

  // Return the transmittance of this material at texture-coordinates COORDS.
  //
  // Note that this method only applies to "simple"
  // transparency/translucency, where transmitted rays don't change
  // direction; materials that are conceptually "transparent," but
  // which exhibit more complex effects like refraction (which change
  // the direction) may return zero from this method.
  //
  virtual Color transmittance (const TexCoords &/*coords*/) const { return 0; }

  // If this is a light-emitting material, call PRIMITIVE's
  // Primitive::add_light method with an appropriate intensity to add a
  // light to LIGHTS (for non-light-emitting materials, do nothing).
  //
  virtual void add_light (const Primitive & /*primitive*/,
			  std::vector<Light *> & /*lights*/)
    const
  {
  }

  Ref<const Tex<float> > bump_map;

  unsigned char flags;
};


}

#endif /* __MATERIAL_H__ */


// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
