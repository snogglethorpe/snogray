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
#include "fresnel.h"

namespace Snogray {

class CookTorrance : public Brdf
{
public:

  CookTorrance (const Color &_spec_col, float _m, const Ior &_ior)
    : specular_color (_spec_col), m (_m), ior (_ior)
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // NUM is only a suggestion.
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const;

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const;

  Color specular_color;

  // Cook Torrance parameters:

  // m:  RMS slope of microfacets -- large m means more spread out
  // reflections.
  //
  float m;

  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;
};

// Source of "constant" (not-to-be-freed) CookTorrance BRDFs
//
extern const CookTorrance *
cook_torrance (const Color &spec_col, float m, const Ior &ior = 1.5);

}

#endif /* __COOK_TORRANCE_H__ */

// arch-tag: 73c818bb-1305-412f-a616-6950b8d9ef39
