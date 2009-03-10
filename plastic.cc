// plastic.h -- Plastic (thin, transmissive, reflective) material
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

#include "brdf.h"
#include "fresnel.h"
#include "intersect.h"

#include "plastic.h"

using namespace snogray;


// Common information used for refraction methods.
//
class PlasticBrdf : public Brdf
{
public:

  PlasticBrdf (const Plastic &_plastic, const Intersect &isec)
    : Brdf (isec), plastic (_plastic)
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (unsigned, IllumSampleVec &samples) const
  {
    // Calculate fresnel surface reflection at the ray angle
    //
    float cos_xmit_angle = isec.cos_n (isec.v);
    float medium_ior = isec.trace.medium.ior;
    float refl = Fresnel (medium_ior, plastic.ior).reflectance (cos_xmit_angle);

    // Render transmitted light (some light is lost due to fresnel reflection
    // from the back surface).
    //
    Color xmit = plastic.color * (1 - refl);

    unsigned old_num_samps = samples.size ();

    // Transmitted sample.
    //
    if (xmit > Eps)
      samples.push_back (
		IllumSample (-isec.v, xmit, 0,
			     IllumSample::SPECULAR|IllumSample::TRANSMISSIVE));

    // Reflected sample.
    //
    if (refl > Eps)
      samples.push_back (
		IllumSample (isec.v.mirror (Vec (0, 0, 1)), refl, 0,
			     IllumSample::SPECULAR|IllumSample::REFLECTIVE));

    return samples.size() - old_num_samps;
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

  const Plastic &plastic;
};


// Return a new BRDF object for this material instantiated at ISEC.
//
Brdf *
Plastic::get_brdf (const Intersect &isec) const
{
  return new (isec) PlasticBrdf (*this, isec);
}


// arch-tag: cd843fe9-2c15-4212-80d7-7e302850c1a7
