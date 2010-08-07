// glow.h -- Constant-color reflectance function
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

#ifndef __GLOW_H__
#define __GLOW_H__

#include "tex.h"

#include "material.h"


namespace snogray {


class Glow : public Material
{
public:

  Glow (const TexVal<Color> &col);
  Glow (const TexVal<Color> &col,
	const Ref<const Material> &_underlying_material);

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;

  // Return the medium of this material (used only for refraction).
  //
  virtual const Medium *medium () const;

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color Le (const Intersect &isec) const;

  // Return true if this material emits light.
  //
  virtual bool emits_light () const { return true; }

  // If this is a light-emitting material, call PRIMITIVE's
  // Primitive::add_light method with an appropriate intensity to add a
  // light to LIGHTS (for non-light-emitting materials, do nothing).
  //
  virtual void add_light (const Primitive &primitive,
			  std::vector<Light *> &lights)
    const;

private:

  // Amount of glow.
  //
  TexVal<Color> color;

  // Material to handled reflected light.  This may be zero, for "emissive
  // only" surfaces.
  //
  Ref<const Material> underlying_material;
};


}

#endif /* __GLOW_H__ */


// arch-tag: d53c41c0-1970-4b3e-9047-2f67dd943922
