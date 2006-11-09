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

  float exponent;
};

// Source of "constant" (not-to-be-freed) Phong BRDFs
//
extern const Phong *phong (const Color &spec_col, float exp);


}

#endif /* __PHONG_H__ */


// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
