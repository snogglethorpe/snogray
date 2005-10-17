// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLASS_H__
#define __GLASS_H__

#include "mirror.h"

namespace Snogray {

class Glass : public Mirror
{
public:

  Glass (Medium _medium, Color reflectance,
	 const Color &col = Color::white, const LightModel &lmodel = lambert)
    : Mirror (reflectance, col, lmodel), medium (_medium)
  { }
  Glass (Medium _medium, Color reflectance,
	 const Color &col = Color::white, float phong_exp)
    : Mirror (reflectance, col, phong_exp), medium (_medium)
  { }

  virtual Color render (const Intersect &isec, TraceState &tstate) const;

  Medium medium;
};

}

#endif /* __GLASS_H__ */

// arch-tag: 4f86bd63-4099-40de-b81b-c5d397002a3e
