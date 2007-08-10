// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "fresnel.h"
#include "intersect.h"

#include "glass.h"

using namespace snogray;

// Common information used for refraction methods.
//
struct Refraction
{
  Refraction (const Glass &glass, const Intersect &isec)
    : entering (! isec.back),
      old_medium (entering ? isec.trace.medium : &glass.medium),
      new_medium (entering ? &glass.medium : isec.trace.enclosing_medium ()),
      old_ior (old_medium ? old_medium->ior : 1),
      new_ior (new_medium ? new_medium->ior : 1)
  { }

  // The proportion of light which will be transmitted towards the viewer.
  // COS_XMIT_ANGLE is the angle between the surface normal and the ray on
  // the other side of the interface.
  //
  float transmittance (float cos_xmit_angle)
  {
    // Calculate the amount of transmitted light which would be lost due
    // to Fresnel reflection from the interface.
    //
    float lost = Fresnel (new_ior, old_ior).reflectance (cos_xmit_angle);

    // The amount of transmited light is anything remaining, with its
    // brightness adjusted to account for the change in solid angle of
    // the light ray.
    //
    return (1 - lost) * (new_ior * new_ior) / (old_ior * old_ior);
  }

  // The proportion of light which will be reflected towards the viewer
  // from the same side of the interface, due to fresnel reflection .
  // COS_REFL_ANGLE is the angle between the surface normal and the ray to
  // be reflected.
  //
  float reflectance (float cos_refl_angle)
  {
    return Fresnel (old_ior, new_ior).reflectance (cos_refl_angle);
  }

  // Are we entering or exiting the medium enclosed by the glass surface?
  //
  bool entering;

  // The old (from which the eye ray is coming) and new media.
  //
  const Medium *old_medium, *new_medium;

  // The indices of refraction of OLD_MEDIUM and NEW_MEDIUM.
  //
  float old_ior, new_ior;
};

Color
Glass::render (const Intersect &isec) const
{
  Refraction refr (*this, isec);

  Color radiance = 0;

  // Render transmitted light

  Vec xmit_dir = isec.ray.dir.refraction (isec.n, refr.old_ior, refr.new_ior);

  if (! xmit_dir.null ())
    {
      float xmit = refr.transmittance (dot (xmit_dir, -isec.n));

      if (xmit > Eps)
	{
	  Ray xmit_ray (isec.pos, xmit_dir);
	  Trace::Type subtrace_type
	    = refr.entering ? Trace::REFRACTION_IN : Trace::REFRACTION_OUT;
	  Trace &sub_trace = isec.subtrace (subtrace_type, refr.new_medium);
	  radiance += xmit * sub_trace.render (xmit_ray);
	}
    }

  // Render reflected light

  float refl = refr.reflectance (isec.nv);

  if (refl > Eps)
    {
      Vec mirror_dir = isec.v.mirror (isec.n);
      Ray mirror_ray (isec.pos, mirror_dir);

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
  Refraction refr (*this, isec);

  // We don't do real refraction because that would invalidate the light
  // direction!  Just do straight "transparency", using a straight-through
  // direction for shadow rays through the glass instead of in the properly
  // refracted direction.  This gives a fairly decent result, although
  // there are obviously no caustics.

  // Use the straight-through angle.
  //
  float xmit = refr.transmittance (dot (light_ray.dir, -isec.n));

  if (xmit > Eps)
    {
      Trace::Type subtrace_type
	= refr.entering ? Trace::SHADOW_REFR_IN : Trace::SHADOW_REFR_OUT;
      Trace &sub_trace = isec.subtrace (subtrace_type, refr.new_medium);
      return sub_trace.shadow (light_ray, light_color * xmit, light);
    }
  else
    return 0;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
