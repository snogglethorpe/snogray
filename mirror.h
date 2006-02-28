// mirror.h -- Mirror (perfectly reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIRROR_H__
#define __MIRROR_H__

#include "material.h"
#include "fresnel.h"

namespace Snogray {

// Material implementing perfect specular reflectance.
//
class Mirror : public Material
{
public:

  Mirror (const Ior &_ior, const Color &_reflectance,
	  const Color &col, const Brdf *brdf)
    : Material (col, brdf), reflectance (_reflectance), ior (_ior)
  { }
  Mirror (const Ior &_ior, const Color &_reflectance,
	  const Color &col = Color::black)
    : Material (col), reflectance (_reflectance), ior (_ior)
  { }

  virtual Color render (const Intersect &isec) const;

  // Renders only the reflection about ISEC, without adding in other components
  //
  Color reflection (const Intersect &isec) const;

  // Amount/color of light reflected; anything else will be passed to the
  // underlying BRDF.
  //
  Color reflectance;

  // Index of refraction for calculating Fresnel reflection
  //
  Ior ior;
};

}

#endif /* __MIRROR_H__ */

// arch-tag: b622d70c-03ff-49ee-a020-2a44ccfcfdb1
