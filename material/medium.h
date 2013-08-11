// medium.h -- Representation of physical medium
//
//  Copyright (C) 2005-2007, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MEDIUM_H
#define SNOGRAY_MEDIUM_H

#include "geometry/coords.h"
#include "color/color.h"

namespace snogray {

// An absorptive medium through which light can travel.
//
class Medium
{
public:

  // _IOR is this medium's index of refraction, _ABSORB is its absorption
  // coefficient (decrease in intensity per unit length).
  //
  Medium (float _ior = 1, const Color &_absorb = 0)
    : ior (_ior), absorption (_absorb)
  { }

  // Return the amount by which light is attenuated by travelling DISTANCE
  // through this medium.
  //
  Color transmittance (dist_t distance) const
  {
    return pow (Color (E), -absorption * distance);
  }

  // Return LIGHT attenuated by travelling DISTANCE through this medium.
  //
  Color attenuate (Color light, dist_t distance) const
  {
    return light * pow (Color (E), -absorption * distance);
  }

  // Index of refraction; controls how light bends when passing between two
  // different media, according to Snell's law.
  //
  float ior;

  // Amount of light absorbed by 1 unit of this medium.  0 is perfectly
  // clear, and 1 perfectly opaque.
  //
  Color absorption;
};

}

#endif /* SNOGRAY_MEDIUM_H */

// arch-tag: f0df2c38-6387-4801-887c-669001eeff61
