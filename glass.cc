// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "glass.h"

#include "intersect.h"
#include "scene.h"

using namespace Snogray;

Color
Glass::render (const Intersect &isec, TraceState &tstate) const
{
  // Render transmission

  TraceState::TraceType subtrace_type;
  const Medium *old_medium, *new_medium;

  if (isec.reverse)
    {
      // Exiting this object

      new_medium = tstate.enclosing_medium ();
      old_medium = &medium;
      subtrace_type = TraceState::REFRACTION_OUT;
    }
  else
    {
      // Entering this object

      new_medium = &medium;
      old_medium = tstate.medium;
      subtrace_type = TraceState::REFRACTION_IN;
    }

  Vec xmit_dir
    = isec.ray.dir.refraction (isec.normal,
			       old_medium ? old_medium->ior : 1,
			       new_medium ? new_medium->ior : 1);

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      TraceState &sub_tstate
	= tstate.subtrace_state (subtrace_type, new_medium, isec.obj);

      // Render the refracted ray, and combine it with any contribution
      // from reflections and surface lighting.
      //
      return
	(1 - reflectance) * sub_tstate.render (xmit_ray)
	+ Mirror::render (isec, tstate);
    }
  else
    // "Total internal reflection"
    //
    return reflection (isec, tstate);
}

// The general sort of shadow this material will cast.  This value
// should never change for a given material, so can be cached.
//
Material::ShadowType
Glass::shadow_type () const
{
  return Material::SHADOW_MEDIUM;
}

// Calculate the shadowing effect of OBJ on LIGHT_RAY (which points at
// the light, not at the object).  The "non-shadowed" light has color
// LIGHT_COLOR; it's also this method's job to find any further
// shadowing surfaces.
//
Color
Glass::shadow (const Obj *obj, const Ray &light_ray, const Color &light_color,
	       TraceState &tstate)
  const
{
  // We don't do real refraction because that would invalidate the light
  // direction!  Just do straight "transparency".

  TraceState &sub_tstate
    = tstate.subtrace_state (TraceState::SHADOW, &medium, obj);

  return sub_tstate.shadow (light_ray, light_color * (1 - reflectance));
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
