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
  Glass (Color _transmittance, float _ior, Color reflectance,
	 const Color &col = Color::white, const LightModel *lmodel = lambert)
    : Mirror (reflectance, col, lmodel),
      transmittance (_transmittance), ior (_ior)
  { }
  Glass (float _transmittance, float _ior,
	 const Color &col = Color::white, const LightModel *lmodel = lambert)
    : Mirror (1 - _transmittance, col, lmodel),
      transmittance (_transmittance), ior (_ior)
  { }
  Glass (Color _transmittance, float _ior, Color reflectance,
	 const Color &col = Color::white, float phong_exp)
    : Mirror (reflectance, col, phong_exp),
      transmittance (_transmittance), ior (_ior)
  { }
  Glass (float _transmittance, float _ior,
	 const Color &col = Color::white, float phong_exp)
    : Mirror (1 - _transmittance, col, phong_exp),
      transmittance (_transmittance), ior (_ior)
  { }

  virtual Color render (const Intersect &isec, TraceState &tstate) const;

  Color transmittance;

  // Index of refraction
  float ior;
};

}

#endif /* __GLASS_H__ */

// arch-tag: 4f86bd63-4099-40de-b81b-c5d397002a3e
