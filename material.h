// material.h -- Surface material datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "pos.h"
#include "vec.h"
#include "color.h"
#include "brdf.h"
#include "phong.h"
#include "lambert.h"
#include "trace-state.h"

namespace Snogray {

class Intersect;
class Scene;

class Material
{
public:

  enum ShadowType { SHADOW_OPAQUE, SHADOW_NONE, SHADOW_MEDIUM };

  // As a convenience, provide a global lookup service for common brdfs.
  //
  static const Lambert *lambert;
  static const Phong *phong (const Color &spec_col, float exp);

  Material (const Color &col, const Brdf *brdf = lambert)
    : color (col), brdf (brdf ? brdf : lambert)
  { }
  Material (const Color &col, const Color &phong_spec_col, float phong_exp)
    : color (col), brdf (phong (phong_spec_col, phong_exp))
  { }
  virtual ~Material ();

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

  Color color;
  const Brdf *brdf;
};

}

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
