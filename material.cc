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



Color
Material::render (const Intersect &isec) const
{
  return isec.illum ();
}

// Shadow LIGHT_RAY, which points to a light with (apparent) color
// LIGHT_COLOR. and return the shadow color.  This is basically like
// the `render' method, but calls the material's `shadow' method
// instead of its `render' method.
//
// Note that this method is only used for `non-opaque' shadows --
// opaque shadows (the most common kind) don't use it!
//
Color
Material::shadow (const Intersect &isec, const Ray &,
		  const Color &, const Light &light)
  const
{
  // This method only gets called if we encounter an opaque surface,
  // casting a "real" shadow.  We can just immediately return black.

  // Set TRACE's shadow-hint so the next time SURFACE will be
  // immediately tried first when calculating shadows.
  //
  isec.trace.shadow_hints[light.num] = isec.surface;

  // Return black.
  //
  return Color (0, 0, 0);
}

// arch-tag: 3d971faa-322c-4479-acf0-effb05aca10a
