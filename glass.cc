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

  // Return the proportion of light which will be transmitted towards the
  // viewer.  COS_XMIT_ANGLE is the angle between the surface normal and
  // the ray on the other side of the interface.
  //
  // This function does not include light concentration due to the changing
  // solid angle of transmitted light rays (use Refraction::magnify for that).
  //
  float transmittance (float cos_xmit_angle)
  {
    // The amount transmitted is one minus the amount of transmitted light
    // which would be lost due to Fresnel reflection from the interface.
    //
    return 1 - Fresnel (new_ior, old_ior).reflectance (cos_xmit_angle);
  }

  // Return the amount of "magnification" due to the change in solid angle of
  // a transmitted the light ray.
  //
  float magnification ()
  {
    return (new_ior * new_ior) / (old_ior * old_ior);
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

  // Direction from which transmitted light comes.
  //
  Vec xmit_dir
    = isec.ray.dir.refraction (isec.normal_frame.z, refr.old_ior, refr.new_ior);

  // Proportion of transmitted light, 0-1.  Note that in the case of total
  // internal reflection, XMIT_DIR will be a null vector (all zeros).
  //
  float xmit
    = (xmit_dir.null()
       ? 0
       : refr.transmittance (dot(xmit_dir, -isec.normal_frame.z)));

  // Proportion of reflected light, 0-1
  //
  float refl = refr.reflectance (isec.cos_n (isec.v));

  // Maybe use a "russian roulette" test to avoid excessive recursion.
  // This test probabilistically terminates further recursion, and scales
  // any successful results to avoid bias.  We only do this past a certain
  // recursion depth, as doing so always gives excessively noisy results.
  //
  if (isec.trace.depth >= isec.trace.global.params.spec_rr_depth)
    {
      // Bound is the upper bound for our random choice; it is normally 1
      // except in rare cases where XMIT+REFL is greater than one, in which
      // case it is XMIT+REFL.
      //
      float bound = max (xmit + refl, 1.f);

      // Random  number used to choose transmission, reflection, or
      // nothing.
      //
      float choice = random (bound);

      // We use the value of CHOICE to decide what to do:
      //
      //  0 - XMIT:  		recurse to handle transmitted light
      //  XMIT - XMIT+REFL:	recurse to handle reflected light
      //  XMIT+REFL - BOUND:	give up and return 0
      //
      // We choose only one of transmission or reflection.
      //
      // To avoid bias due to cases where we terminate without recursing,
      // we scale the result of the recursion we choose by an additional
      // scale factor of 1 / RECURSION_PROBABILITY.  As the probability of
      // choosing transmission is just XMIT / BOUND, and choosing
      // reflection is REFL / BOUND, the 1 / RECURSION_PROBABILITY factor
      // exactly cancels the ordinary scale factor (XMIT or REFL
      // respectively), so the total scale factor becomes simply BOUND.
      //
      if (choice < xmit)
	{
	  refl = 0;		// don't recurse for reflection
	  xmit = bound;		// avoid RR bias in transmission
	}
      else if (choice < xmit + refl)
	{
	  xmit = 0;		// don't recurse for transmission
	  refl = bound;		// avoid RR bias in reflection
	}
      else
	return 0;		// don't recurse at all
    }

  Color radiance = 0;

  // Maybe recurse to render transmitted light.
  //
  if (xmit > Eps)
    {
      Ray xmit_ray (isec.normal_frame.origin, xmit_dir);
      Trace::Type subtrace_type
	= refr.entering ? Trace::REFRACTION_IN : Trace::REFRACTION_OUT;
      Trace &sub_trace = isec.subtrace (subtrace_type, refr.new_medium);
      radiance += xmit * refr.magnification() * sub_trace.render (xmit_ray);
    }

  // Maybe recurse to render reflected light.
  //
  if (refl > Eps)
    {
      Vec eye_dir = -isec.ray.dir;
      Vec mirror_dir = eye_dir.mirror (isec.normal_frame.z);
      Ray mirror_ray (isec.normal_frame.origin, mirror_dir);

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
  float xmit = refr.transmittance (dot (light_ray.dir, -isec.normal_frame.z));

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
