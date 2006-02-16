// material.cc -- Surface material datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "material.h"

#include "scene.h"
#include "intersect.h"
#include "lambert.h"
#include "phong.h"

using namespace Snogray;

Material::~Material () { } // stop gcc bitching



Color
Material::render (const Intersect &isec, TraceState &tstate) const
{
  return tstate.illum (isec, color, *brdf);
}

// The general sort of shadow this material will cast.  This value
// should never change for a given material, so can be cached.
//
Material::ShadowType
Material::shadow_type () const
{
  return Material::SHADOW_OPAQUE;
}

// Calculate the shadowing effect of SURFACE on LIGHT_RAY (which points
// at the light, not at the surface).  The "non-shadowed" light has
// color LIGHT_COLOR; it's also this method's job to find any further
// shadowing surfaces.
//
Color
Material::shadow (const Surface *surface,
		  const Ray &light_ray, const Color &light_color,
		  const Light &light, TraceState &tstate)
  const
{
  // This method only gets called if we encounter an opaque surface,
  // casting a "real" shadow.  We can just immediately return black.

  // Set TSTATE's shadow-hint so the next time SURFACE will be
  // immediately tried first when calculating shadows.
  //
  tstate.shadow_hints[light.num] = surface;

  // Return black.
  //
  return Color (0, 0, 0);
}

// arch-tag: 3d971faa-322c-4479-acf0-effb05aca10a
