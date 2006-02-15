// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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
	 const Color &col = Color::white, const Brdf *brdf = lambert)
    : Mirror (reflectance, col, brdf), medium (_medium)
  { }
  Glass (Medium _medium, Color reflectance,
	 const Color &col, const Color &spec_col, float phong_exp = 1)
    : Mirror (reflectance, col, spec_col, phong_exp), medium (_medium)
  { }

  virtual Color render (const Intersect &isec, TraceState &tstate) const;

  // The general sort of shadow this material will cast.  This value
  // should never change for a given material, so can be cached.
  //
  virtual ShadowType shadow_type () const;

  // Calculate the shadowing effect of SURFACE on LIGHT_RAY (which points at
  // the light, not at the surface).  The "non-shadowed" light has color
  // LIGHT_COLOR; it's also this method's job to find any further
  // shadowing surfaces.
  //
  virtual Color shadow (const Surface *surface,
			const Ray &light_ray, const Color &light_color,
			const Light &light, TraceState &tstate)
    const;

  Medium medium;
};

}

#endif /* __GLASS_H__ */

// arch-tag: 4f86bd63-4099-40de-b81b-c5d397002a3e
