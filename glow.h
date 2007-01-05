// glow.h -- Constant-color reflectance function
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLOW_H__
#define __GLOW_H__

#include "material.h"

namespace snogray {

class Glow : public Material
{
public:

  Glow (const Color &_color) : Material (_color, Material::SHADOW_NONE) { }

  virtual Color render (const Intersect &isec) const;

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
};

}

#endif /* __GLOW_H__ */

// arch-tag: d53c41c0-1970-4b3e-9047-2f67dd943922
