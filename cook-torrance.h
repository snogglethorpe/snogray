// cook-torrance.h -- Cook-Torrance reflectance function
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COOK_TORRANCE_H__
#define __COOK_TORRANCE_H__

#include "brdf.h"

namespace Snogray {

class CookTorrance : public Brdf
{
public:

  CookTorrance (const Color &_spec_col,
		float _m, float _ior, float _ior_imag = 0)
    : specular_color (_spec_col),
      m (_m), m_2_inv (1 / (m * m)), ior (_ior), ior_imag (_ior_imag)
  { }

  // Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
  // For best results, they should be distributed according to the BRDF's
  // importance function.
  //
  virtual void gen_samples (const Intersect &isec, SampleRayVec &samples)
    const;

  // Modify the value of each of the light-samples in SAMPLES according to
  // the BRDF's reflectivity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const;

  Color specular_color;

  // Cook Torrance parameters:

  // m:  "RMS slope of microfacets -- large m means more spread out
  // reflections".  We actually store 1 / m^2, as that's what the
  // calculations use.
  //
  float m;
  float m_2_inv;

  // Index of refraction.  Used for calculating fresnel reflection term.
  // For metals, the IOR also has an imaginary term.
  //
  float ior, ior_imag;
};

// Source of "constant" (not-to-be-freed) CookTorrance BRDFs
//
extern const CookTorrance *
cook_torrance (const Color &spec_col, float m, float ior = 1.5,
	       float ior_imag = 0);

}

#endif /* __COOK_TORRANCE_H__ */

// arch-tag: 73c818bb-1305-412f-a616-6950b8d9ef39
