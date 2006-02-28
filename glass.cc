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

  float old_ior = old_medium ? old_medium->ior : 1;
  float new_ior = new_medium ? new_medium->ior : 1;

  Vec xmit_dir = isec.ray.dir.refraction (isec.normal, old_ior, new_ior);

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      // Internal reflection
      //
      float fres_refl
	= Fresnel (old_ior, new_ior).reflectance (dot (xmit_dir, -isec.normal));
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

  // ---HACK---HACK---HACK---HACK---HACK---HACK---HACK---HACK---HACK
  // Gack, our caller doesn't give us enough info!!  Kludge something up.
  // [We should rework the transparent shadow interface so that the
  // intersection is passed down just like the normal render method.]
  // ---HACK---HACK---HACK---HACK---HACK---HACK---HACK---HACK---HACK

  IsecParams fake_isec_params;
  fake_isec_params.u = fake_isec_params.v = 0;

  Ray fake_incoming_ray (light_ray.origin - light_ray.dir, light_ray.origin,
			 light_ray.len);

  Intersect fake_isec
    = surface->intersect_info (fake_incoming_ray, fake_isec_params, trace);

  // Determine whether the ray is entering or exiting this glass (note that
  // this is the opposite of the calculation in the Glass::render method,
  // as the light ray is coming the opposit direction).
  //
  const Medium *old_medium, *new_medium;
  if (fake_isec.back)
    {
      // Light-ray _enters_ this glass
      new_medium = &medium;
      old_medium = fake_isec.trace.medium;
    }
  else
    {
      // Light-ray _exits_ this glass
      new_medium = fake_isec.trace.enclosing_medium ();
      old_medium = &medium;
    }

  float old_ior = old_medium ? old_medium->ior : 1;
  float new_ior = new_medium ? new_medium->ior : 1;

  float fres_refl
    = Fresnel (old_ior, new_ior).reflectance (
				   dot (light_ray.dir, fake_isec.normal));
  float xmit = 1 - fres_refl;
      
  if (xmit > Eps)
    {
      Trace &sub_trace = trace.subtrace (Trace::SHADOW, new_medium, surface);
      return sub_trace.shadow (light_ray, light_color * xmit, light);
    }
  else
    return 0;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
