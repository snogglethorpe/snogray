// glass.h -- Glass (transmissive, reflective) material
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
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
      old_ior ((entering ? isec.trace.medium : glass._medium).ior),
      new_ior ((entering ? glass._medium : isec.trace.enclosing_medium ()).ior)
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (unsigned, IllumSampleVec &samples) const
  {
    // Direction from which transmitted light comes.
    //
    Vec xmit_dir = (-isec.v).refraction (Vec (0, 0, 1), old_ior, new_ior);

    // Proportion of transmitted light, 0-1.  Note that in the case of total
    // internal reflection, XMIT_DIR will be a null vector (all zeros).
    //
    float xmit = (xmit_dir.null() ? 0 : transmittance (-isec.cos_n (xmit_dir)));

    // Proportion of reflected light, 0-1
    //
    float refl = reflectance (isec.cos_n (isec.v));

    // Transmitted sample.
    //
    if (xmit > Eps)
      samples.push_back (
		IllumSample (xmit_dir, xmit, 0,
			     IllumSample::SPECULAR|IllumSample::TRANSMISSIVE));

    // Reflected sample.
    //
    if (refl > Eps)
      samples.push_back (
		IllumSample (isec.v.mirror (Vec (0, 0, 1)), refl, 0,
			     IllumSample::SPECULAR|IllumSample::REFLECTIVE));

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
