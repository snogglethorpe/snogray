// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005-2010, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "fresnel.h"
#include "intersect.h"
#include "media.h"
#include "bsdf.h"

#include "glass.h"


using namespace snogray;


// Common information used for refraction methods.
//
class snogray::GlassBsdf : public Bsdf
{
public:

  GlassBsdf (const Glass &glass, const Intersect &isec)
    : Bsdf (isec), entering (! isec.back),
      old_ior ((entering ? isec.media.medium : glass._medium).ior),
      new_ior ((entering
		? glass._medium
		: isec.media.enclosing_medium (isec.context.default_medium))
	       .ior)
  { }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    if (flags & SPECULAR)
      {
	// Clear all but the direction flags.  This means it will be
	// either REFLECTIVE, TRANSMISSIVE, or REFLECTIVE|TRANSMISSIVE.
	//
	flags &= ALL_DIRECTIONS;

	// Direction from which transmitted light comes.
	//
	Vec xmit_dir = (-isec.v).refraction (Vec (0, 0, 1), old_ior, new_ior);

	// The cosine of the angle between the transmitted ray and the
	// reverse-surface-normal (on the transmission side of the
	// material).
	//
	// Since that angle is 180 minus the angle with the front-surface
	// normal, we just calculate the cosine of the latter instead, and
	// then negate it, as cos (180 - theta) = -cos (theta).
	//
	// In the case of total internal reflection, XMIT_DIR will be a null
	// vector, which will cause Intersect::cos_n to return zero.
	//
	float cos_xmit_angle = -isec.cos_n (xmit_dir);

	// The cosine of the angle between the reflected ray and the surface
	// normal.  For reflection this angle is the same as the angle
	// between the view ray and the normal.
	//
	float cos_refl_angle = abs (isec.cos_n (isec.v));

	// Proportion of transmitted light.
	//
	float xmit = (cos_xmit_angle == 0) ? 0 : transmittance (cos_xmit_angle);

	// Proportion of reflected light.
	//
	float refl = (cos_refl_angle == 0) ? 0 : reflectance (cos_refl_angle);

	if (xmit + refl != 0)
	  {
	    // Probability we will choose the transmissive direction.
	    // If the user forced the choice by only passing one of
	    // TRANSMISSIVE or REFLECTIVE flags, then the probability will
	    // be 0 or 1 respectively.
	    //
	    float xmit_probability
	      = (flags == TRANSMISSIVE ? 1
		 : flags == REFLECTIVE ? 0
		 : xmit / (xmit + refl));

	    // Choose between the two possible directions based on their
	    // relative strengths.
	    //
	    // We also add the appropriate 1 / cos (theta_i) term just
	    // before returning (so we only need do one expensive division).
	    //
	    if (param.u < xmit_probability)
	      // Transmitted sample.
	      {
		if (cos_xmit_angle != 0)
		  xmit /= cos_xmit_angle;

		return Sample (xmit, xmit_probability, xmit_dir,
			       SPECULAR|TRANSMISSIVE);
	      }
	    else
	      // Reflected sample.
	      {
		if (cos_refl_angle != 0)
		  refl /= cos_refl_angle;

		return Sample (refl, (1 - xmit_probability),
			       isec.v.mirror (Vec (0, 0, 1)),
			       SPECULAR|REFLECTIVE);
	      }
	  }
      }

    return Sample ();
  }

  // Evaluate this BSDF in direction DIR, and return its value and pdf.
  // If FLAGS is specified, then only the given types of surface
  // interaction are considered.
  //
  virtual Value eval (const Vec &, unsigned) const
  {
    return Value ();		// we're specular, so all samples fail
  }

  // Return a bitmask of flags from Bsdf::Flags, describing what
  // types of scatting this BSDF supports.  The returned value will
  // include only flags in LIMIT (default, all flags).
  //
  // The various fields (Bsdf::ALL_LAYERS, Bsdf::ALL_DIRECTIONS) in
  // the returned value should be consistent -- a layer flag like
  // Bsdf::DIFFUSE should only be included if that layer is
  // supported by one of the sample-directions
  // (e.g. Bsdf::REFLECTIVE) in the return value, and vice-versa.
  //
  virtual unsigned supports (unsigned limit) const
  {
    return
      (limit & SPECULAR)
      ? (TRANSMISSIVE | REFLECTIVE | SPECULAR) & limit
      : 0;
  }

private:

  // Return the proportion of light which will be transmitted towards the
  // viewer.  COS_XMIT_ANGLE is the angle between the surface normal and
  // the ray on the other side of the interface.
  //
  // This function does not include light concentration due to the changing
  // solid angle of transmitted light rays (use Refraction::magnify for that).
  //
  float transmittance (float cos_xmit_angle) const
  {
    // The amount transmitted is one minus the amount of transmitted light
    // which would be lost due to Fresnel reflection from the interface.
    //
    return 1 - Fresnel (new_ior, old_ior).reflectance (cos_xmit_angle);
  }

  // Return the amount of "magnification" due to the change in solid angle of
  // a transmitted the light ray.
  //
  float magnification () const
  {
    return (new_ior * new_ior) / (old_ior * old_ior);
  }

  // The proportion of light which will be reflected towards the viewer
  // from the same side of the interface, due to fresnel reflection .
  // COS_REFL_ANGLE is the angle between the surface normal and the ray to
  // be reflected.
  //
  float reflectance (float cos_refl_angle) const
  {
    return Fresnel (old_ior, new_ior).reflectance (cos_refl_angle);
  }

  // Are we entering or exiting the medium enclosed by the glass surface?
  //
  bool entering;

  // The indices of refraction of OLD_MEDIUM and NEW_MEDIUM.
  //
  float old_ior, new_ior;
};


// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
Glass::get_bsdf (const Intersect &isec) const
{
  return new (isec) GlassBsdf (*this, isec);
}


// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
