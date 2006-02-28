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
Glass::render (const Intersect &isec) const
{
  // Render transmission

  Trace::TraceType subtrace_type;
  const Medium *old_medium, *new_medium;

  if (isec.back)
    {
      // Exiting this surface

      new_medium = isec.trace.enclosing_medium ();
      old_medium = &medium;
      subtrace_type = Trace::REFRACTION_OUT;
    }
  else
    {
      // Entering this surface

      new_medium = &medium;
      old_medium = isec.trace.medium;
      subtrace_type = Trace::REFRACTION_IN;
    }

  Vec xmit_dir
    = isec.ray.dir.refraction (isec.normal,
			       old_medium ? old_medium->ior : 1,
			       new_medium ? new_medium->ior : 1);

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      Trace &sub_trace
	= isec.trace.subtrace (subtrace_type, new_medium, isec.surface);

      // Render the refracted ray, and combine it with any contribution
      // from reflections and surface lighting.
      //
      return
	(1 - reflectance) * sub_trace.render (xmit_ray)
	+ Mirror::render (isec);
    }
  else
    // "Total internal reflection"
    //
    return reflection (isec);
}

// The general sort of shadow this material will cast.  This value
// should never change for a given material, so can be cached.
//
Material::ShadowType
Glass::shadow_type () const
{
  return Material::SHADOW_MEDIUM;
}

// Calculate the shadowing effect of SURFACE on LIGHT_RAY (which points at
// the light, not at the surface).  The "non-shadowed" light has color
// LIGHT_COLOR; it's also this method's job to find any further
// shadowing surfaces.
//
Color
Glass::shadow (const Surface *surface,
	       const Ray &light_ray, const Color &light_color,
	       const Light &light, Trace &trace)
  const
{
  // We don't do real refraction because that would invalidate the light
  // direction!  Just do straight "transparency".

  Trace &sub_trace
    = trace.subtrace (Trace::SHADOW, &medium, surface);

  return sub_trace.shadow (light_ray, light_color * (1 - reflectance), light);
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
