// mirror.h -- Mirror (perfectly reflective) material
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

#ifndef SNOGRAY_MIRROR_H
#define SNOGRAY_MIRROR_H

#include "texture/tex.h"
#include "material.h"
#include "fresnel.h"


namespace snogray {


// A Material implementing perfect specular reflection.  It is
// structured as a specularly reflecting layer on top of another
// material, which handles any light that gets throught the reflecting
// layer.  The reflecting layer may be a dielectic (like glass) with a
// non-complex index of refraction, in which case it will only reflect
// at some angles, or a conductor (whose index of refraction will also
// have an an extinction coefficient k), which will reflect at all
// angles.
//
class Mirror : public Material
{
public:

  Mirror (const Ior &_ior, const TexVal<Color> &_reflectance,
	  const Ref<const Material> &_underlying_material)
    : ior (_ior), reflectance (_reflectance),
      underlying_material (_underlying_material)
  {
    bump_map = _underlying_material->bump_map;
  }

  // A mirror with a simple lambertian underlying material.
  //
  Mirror (const Ior &_ior,
	  const TexVal<Color> &_reflectance,
	  const TexVal<Color> &col = Color(0));

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;


  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;

  // Amount of light reflected (further modulated by a fresnel reflection
  // term using IOR).
  //
  TexVal<Color> reflectance;

  // MATERIAL underlying the mirror coating, which does the real work.
  // This may be zero, for perfectly black surfaces.
  //
  Ref<const Material> underlying_material;
};


}

#endif /* SNOGRAY_MIRROR_H */


// arch-tag: b622d70c-03ff-49ee-a020-2a44ccfcfdb1
