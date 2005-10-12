// medium.h -- Representation of physical medium
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MEDIUM_H__
#define __MEDIUM_H__

#include "space.h"
#include "color.h"

namespace Snogray {

// A medium through which light can travel
//
class Medium
{
public:

  Medium (Color _xmit, float _ior = 1) : transmittance (_xmit), ior (_ior) { }

  // Return LIGHT attenuated by travelling DISTANCE through this medium.
  //
  Color attenuate (Color light, dist_t distance) const
  {
    return light * transmittance.pow (distance);
  }

  // Amount of light left after passing through 1 unit of this medium.
  // Color::white is perfectly clear, and Color::black, perfectly opaque;
  // values brighter than Color::white will have the effect of
  // _intensifying_ passing through them!
  //
  Color transmittance;

  // Index of refraction; controls how light bends when passing between two
  // different media, according to Snell's law.
  //
  float ior;
};

}

#endif /* __MEDIUM_H__ */

// arch-tag: f0df2c38-6387-4801-887c-669001eeff61
