// glow.h -- Constant-color lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

namespace Snogray {

class Glow : public Material
{
public:
  Glow (const Color &_color) : Material (_color) { }

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
			TraceState &tstate)
    const;
};

}

#endif /* __GLOW_H__ */

// arch-tag: d53c41c0-1970-4b3e-9047-2f67dd943922
