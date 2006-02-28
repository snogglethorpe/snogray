// phog.h -- Phong reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHONG_H__
#define __PHONG_H__

#include "brdf.h"

namespace Snogray {

class Phong : public Brdf
{
public:

  Phong (const Color &_spec_col, float _exponent)
    : specular_color (_spec_col), exponent (_exponent)
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

  float exponent;
};

// Source of "constant" (not-to-be-freed) Phong BRDFs
//
extern const Phong *phong (const Color &spec_col, float exp);

}

#endif /* __PHONG_H__ */

// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
