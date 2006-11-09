// lambert.h -- Lambertian reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LAMBERT_H__
#define __LAMBERT_H__

#include "brdf.h"

namespace Snogray {

class Lambert : public Brdf
{
public:

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
};

// There's only one possible lambert object so create it here.
//
extern const Lambert *lambert;

}

#endif /* __LAMBERT_H__ */

// arch-tag: ca8981f3-5471-4e8a-ac8b-2e3e54c83b64
