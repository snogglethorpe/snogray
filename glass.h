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

#include "material.h"
#include "medium.h"

namespace Snogray {

class Glass : public Material
{
public:

  Glass (Medium _medium) : Material (1), medium (_medium) { }

  virtual Color render (const Intersect &isec) const;

  // The general sort of shadow this material will cast.  This value
  // should never change for a given material, so can be cached.
  //
  virtual ShadowType shadow_type () const;

  // Shadow LIGHT_RAY, which points to a light with (apparent) color
  // LIGHT_COLOR. and return the shadow color.  This is basically like
  // the `render' method, but calls the material's `shadow' method
  // instead of its `render' method.
  //
  // Note that this method is only used for `non-opaque' shadows --
  // opaque shadows (the most common kind) don't use it!
  //
  virtual Color shadow (const Intersect &isec, const Ray &light_ray,
			const Color &light_color, const Light &light)
    const;

  Medium medium;
};

}

#endif /* __GLASS_H__ */

// arch-tag: 4f86bd63-4099-40de-b81b-c5d397002a3e
