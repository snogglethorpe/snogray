// brdf.h -- Bi-directional reflectance distribution functions
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BRDF_H__
#define __BRDF_H__

#include <vector>

#include "pos.h"
#include "vec.h"
#include "color.h"
#include "illum-sample.h"

namespace snogray {

class Intersect;

class Brdf
{
public:

  virtual ~Brdf () { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const = 0;

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const = 0;
};

}

#endif /* __BRDF_H__ */

// arch-tag: 8360ddd7-dc17-40b8-8319-8f6d61fe62bf
