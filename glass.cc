// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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
#include "brdf.h"

#include "glass.h"


using namespace snogray;


// Common information used for refraction methods.
//
class snogray::GlassBrdf : public Brdf
{
public:

  GlassBrdf (const Glass &glass, const Intersect &isec)
    : Brdf (isec), entering (! isec.back),
      old_ior ((entering ? isec.media.medium : glass._medium).ior),
      new_ior ((entering
		? glass._medium
		: isec.media.enclosing_medium (isec.context.default_medium))
	       .ior)
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (unsigned, IllumSampleVec &samples) const
  {
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
    float cos_refl_angle = isec.cos_n (isec.v);

    if (cos_xmit_angle != 0)
      {
	// Proportion of transmitted light.
	//
	float xmit = transmittance (cos_xmit_angle) / cos_xmit_angle;

	// Transmitted sample.
	//
	if (xmit > Eps)
	  samples.push_back (
		    IllumSample (xmit_dir, xmit, 0,
				 IllumSample::SPECULAR
				 |IllumSample::TRANSMISSIVE));
      }

    if (cos_refl_angle != 0)
      {
	// Proportion of reflected light.
	//
	float refl = reflectance (cos_refl_angle) / cos_refl_angle;

	// Reflected sample.
	//
	if (refl > Eps)
	  samples.push_back (
		    IllumSample (isec.v.mirror (Vec (0, 0, 1)), refl, 0,
				 IllumSample::SPECULAR
				 |IllumSample::REFLECTIVE));
      }

    // All our samples are specular, which "don't count".
    //
    return 0;
  }

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const
  {
    for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
      s->brdf_val = 0;
  }

  // Return a sample of this BRDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    if (flags & SPECULAR)
      {
	// Clear all but the direction flags.  This means it will be
	// either REFLECTIVE, TRANSMISSIVE, or REFLECTIVE|TRANSMISSIVE.
	//
	flags &= SAMPLE_DIR;

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
	float cos_refl_angle = isec.cos_n (isec.v);

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

  // Evaluate this BRDF in direction DIR, and return its value and pdf.
  //
  virtual Value eval (const Vec &) const
  {
    return Value ();		// we're specular, so all samples fail
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


// Return a new BRDF object for this material instantiated at ISEC.
//
Brdf *
Glass::get_brdf (const Intersect &isec) const
{
  return new (isec) GlassBrdf (*this, isec);
}


// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
