// mirror.h -- Mirror (perfectly reflective) material
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIRROR_H__
#define __MIRROR_H__

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

  Mirror (const Ior &_ior, const Color &_reflectance,
	  const Material *_underlying_material,
	  bool _free_underlying_material = false)
    : ior (_ior), reflectance (_reflectance),
      underlying_material (_underlying_material),
      free_underlying_material (_free_underlying_material)
  { }

  // A mirror with a simple lambertian underlying material.
  //
  Mirror (const Ior &_ior, const Color &_reflectance, const Color &col = 0);

  ~Mirror ()
  {
    if (free_underlying_material)
      delete underlying_material;
  }

  virtual Color render (const Intersect &isec) const;

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &isec) const;


  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;

  // Amount of light reflected (further modulated by a fresnel reflection
  // term using IOR).
  //
  Color reflectance;

  // MATERIAL underlying the mirror coating, which does the real work.
  // This may be zero, for perfectly black surfaces.
  //
  const Material *underlying_material;

  // If true, free UNDERLYING_MATERIAL when destroyed.
  //
  bool free_underlying_material;
};


}

#endif /* __MIRROR_H__ */


// arch-tag: b622d70c-03ff-49ee-a020-2a44ccfcfdb1
