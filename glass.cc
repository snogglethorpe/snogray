// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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

  Trace::Type subtrace_type;
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

  float old_ior = old_medium ? old_medium->ior : 1;
  float new_ior = new_medium ? new_medium->ior : 1;

  Vec xmit_dir = isec.ray.dir.refraction (isec.normal, old_ior, new_ior);

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      // Internal reflection.  Note that the indices of refraction are
      // reversed compared to the calculation of XMIT_DIR above; this is
      // because we're calculating the reflection of light _coming_ from
      // that direction.
      //
      float fres_refl
	= Fresnel (new_ior, old_ior).reflectance (dot (xmit_dir, -isec.normal));
      float xmit = 1 - fres_refl;
      
      if (xmit > Eps)
	{
	  Trace &sub_trace
	    = isec.trace.subtrace (subtrace_type, new_medium, isec.surface);

	  // Render the refracted ray, and combine it with any contribution
	  // from reflections and surface lighting.
	  //
	  return xmit * sub_trace.render (xmit_ray) + Mirror::render (isec);
	}
      else
	return Mirror::render (isec);
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

// Shadow LIGHT_RAY, which points to a light with (apparent) color
// LIGHT_COLOR. and return the shadow color.  This is basically like
// the `render' method, but calls the material's `shadow' method
// instead of its `render' method.
//
// Note that this method is only used for `non-opaque' shadows --
// opaque shadows (the most common kind) don't use it!
//
Color
Glass::shadow (const Intersect &isec, const Ray &light_ray,
	       const Color &light_color, const Light &light)
  const
{
  // We don't do real refraction because that would invalidate the light
  // direction!  Just do straight "transparency".

  Trace::Type subtrace_type;
  const Medium *old_medium, *new_medium;

  if (isec.back)
    {
      // Exiting this surface

      new_medium = isec.trace.enclosing_medium ();
      old_medium = &medium;
      subtrace_type = Trace::SHADOW_REFR_OUT;
    }
  else
    {
      // Entering this surface

      new_medium = &medium;
      old_medium = isec.trace.medium;
      subtrace_type = Trace::SHADOW_REFR_IN;
    }

  float old_ior = old_medium ? old_medium->ior : 1;
  float new_ior = new_medium ? new_medium->ior : 1;

  float fres_refl
    = Fresnel (new_ior, old_ior).reflectance (dot (light_ray.dir, -isec.normal));
  float xmit = 1 - fres_refl;
      
  if (xmit > Eps)
    {
      Trace &sub_trace
	= isec.trace.subtrace (subtrace_type, new_medium, isec.surface);
      return sub_trace.shadow (light_ray, light_color * xmit, light);
    }
  else
    return 0;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
