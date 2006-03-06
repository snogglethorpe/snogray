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

#include "fresnel.h"
#include "intersect.h"

#include "glass.h"

using namespace Snogray;

// Common information used for reflection.
//
struct Refraction
{
  Refraction (const Intersect &isec)
    : old_medium (isec.trace.medium), new_medium (&medium),
      subtrace_type (Trace::REFRACTION_IN)
  {
    // We initially assumed we're entering the enclosed medium, but if we
    // struck the back-face of the surface, then we must be exiting
    // instead.
    //
    if (isec.back)
      {
	new_medium = isec.trace.enclosing_medium ();
	old_medium = &medium;
	subtrace_type = Trace::REFRACTION_OUT;
      }

    old_ior = old_medium ? old_medium->ior : 1;
    new_ior = new_medium ? new_medium->ior : 1;
  }

  float transmittance (float cos_xmit_angle)
  {
    return ((1 - Fresnel (new_ior, old_ior).reflectance (cos_xmit_angle))
	    * (new_ior * new_ior) / (old_ior * old_ior));
  }

  float reflectance (float cos_refl_angle)
  {
    return Fresnel (old_ior, new_ior).reflectance (cos_refl_angle);
  }

  const Medium *old_medium, *new_medium;

  float old_ior, new_ior;

  Trace::Type subtrace_type;
};

Color
Glass::render (const Intersect &isec) const
{
  Refraction refr (isec);

  Color radiance;

  // Render transmitted light

  Vec xmit_dir = isec.ray.dir.refraction (isec.normal, refr.old_ior, refr.new_ior);
  if (! xmit_dir.null ())
    {
      float xmit = refr.transmittance (dot (xmit_dir, -isec.normal));

      if (xmit > Eps)
	{
	  Ray xmit_ray (isec.point, xmit_dir);
	  Trace &sub_trace = isec.subtrace (subtrace_type, new_medium);
	  radiance += xmit * sub_trace.render (xmit_ray);
	}
    }

  // Render reflected light

  float refl = refr.reflectance (dot (isec.normal, isec.viewer));

  if (refl > Eps)
    {
      Vec mirror_dir = isec.ray.dir.reflection (isec.normal);
      Ray mirror_ray (isec.point, mirror_dir);

      radiance += refl * isec.subtrace (Trace::REFLECTION).render (mirror_ray);
    }

  return radiance;
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

  Refraction refr (isec);

  float xmit = refr.reflectance (dot (light_ray.dir, -isec.normal));
      
  if (xmit > Eps)
    {
      Trace &sub_trace = isec.subtrace (subtrace_type, new_medium);
      return sub_trace.shadow (light_ray, light_color * xmit, light);
    }
  else
    return 0;
}

// The general sort of shadow this material will cast.  This value
// should never change for a given material, so can be cached.
//
Material::ShadowType
Glass::shadow_type () const
{
  return Material::SHADOW_MEDIUM;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
