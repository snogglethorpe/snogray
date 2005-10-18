// glow.cc -- Constant-color lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "glow.h"
#include "scene.h"		// for inlined TraceState::shadow method

using namespace Snogray;

Color
Glow::render (const Intersect &isec, TraceState &tstate) const
{
  return color;
}

// The general sort of shadow this material will cast.  This value
// should never change for a given material, so can be cached.
//
Material::ShadowType
Glow::shadow_type () const
{
  return Material::SHADOW_NONE;
}

// Calculate the shadowing effect of OBJ on LIGHT_RAY (which points at
// the light, not at the object).  The "non-shadowed" light has color
// LIGHT_COLOR; it's also this method's job to find any further
// shadowing surfaces.
//
Color
Glow::shadow (const Obj *obj, const Ray &light_ray, const Color &light_color,
	      TraceState &tstate)
  const
{
  // Just pass straight through
  //
  TraceState &sub_tstate = tstate.subtrace_state (TraceState::SHADOW, obj);
  return sub_tstate.shadow (light_ray, light_color);
}

// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
