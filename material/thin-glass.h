// thin-glass.h -- ThinGlass (thin, transmissive, reflective) material
//
//  Copyright (C) 2005-2007, 2009-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_THIN_GLASS_H
#define SNOGRAY_THIN_GLASS_H

#include "material.h"
#include "medium.h"

namespace snogray {

// "ThinGlass" is like our Glass material, but doesn't contain a medium, so
// all filtering effects take place during the surface transition.  This is
// a better match for typical "transparent" materials in many scene
// definition languages than Glass.
//
class ThinGlass : public Material
{
public:

  ThinGlass (const Color &col, float _ior = 1.5)
    : Material (PARTIALLY_OCCLUDING), color (col), ior (_ior)
  { }
  ThinGlass (float _ior = 1.5)
    : color (1), ior (_ior)
  { }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

  // Return the transmittance of this material at the intersection
  // described by ISEC_INFO in medium MEDIUM.
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

  Color color;

  // The index of refraction here is only used for calculating surface
  // reflectance.
  //
  float ior;
};

}

#endif /* SNOGRAY_THIN_GLASS_H */

// arch-tag: eb5d0a02-3751-421d-9ddd-5daf10d56a1e
