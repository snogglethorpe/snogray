// medium.h -- Representation of physical medium
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

#ifndef __MEDIUM_H__
#define __MEDIUM_H__

#include "coords.h"
#include "color.h"

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

#endif /* __MEDIUM_H__ */

// arch-tag: f0df2c38-6387-4801-887c-669001eeff61
