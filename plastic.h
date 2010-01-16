// plastic.h -- Plastic (thin, transmissive, reflective) material
//
//  Copyright (C) 2005, 2006, 2007, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PLASTIC_H__
#define __PLASTIC_H__

#include "material.h"
#include "medium.h"

namespace snogray {

// "Plastic" is like our Glass material, but doesn't contain a medium, so
// all filtering effects take place during the surface transition.  This is
// a better match for typical "transparent" materials in many scene
// definition languages than Glass.
//
class Plastic : public Material
{
public:

  Plastic (const Color &col, float _ior = 1.5)
    : color (col), ior (_ior)
  { }
  Plastic (float _ior = 1.5)
    : color (1), ior (_ior)
  { }

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &isec) const;

  Color color;

  // The index of refraction here is only used for calculating surface
  // reflectance.
  //
  float ior;
};

}

#endif /* __PLASTIC_H__ */

// arch-tag: eb5d0a02-3751-421d-9ddd-5daf10d56a1e
