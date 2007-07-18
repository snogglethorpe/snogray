// glow.cc -- Constant-color reflectance function
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "glow.h"
#include "scene.h"		// for inlined Trace::shadow method

using namespace snogray;

Color
Glow::render (const Intersect &) const
{
  return color;
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
Glow::shadow (const Intersect &isec, const Ray &light_ray,
	      const Color &light_color, const Light &light)
  const
{
  // Just pass straight through
  //
  Trace &sub_trace = isec.subtrace (Trace::SHADOW);
  return sub_trace.shadow (light_ray, light_color, light);
}

// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
