// mis-illum.h -- Illuminator using multiple importance sampling
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIS_ILLUM_H__
#define __MIS_ILLUM_H__

#include "sample-illum.h"

namespace snogray {


class MisIllum : public SampleIllum
{
public:

  MisIllum (const Trace &trace) : SampleIllum (trace) { }

  // Generate samples for estimating the illumination at ISEC.  The
  // samples should be appended to SAMPLES.  NUM_BRDF_SAMPLES is the
  // number of brdf samples to use, and LIGHT_PARAMS is an array of
  // sampling parameters for each light (which may be modified).
  //
  virtual void gen_samples (const Intersect &isec,
			    unsigned num_brdf_samples,
			    std::vector<LightParams> &light_params,
			    IllumSampleVec &samples);
};

class MisIllumGlobalState : public IllumGlobalState
{
public:

  virtual Illum *get_illum (Trace &trace) { return new MisIllum (trace); }
  virtual void put_illum (Illum *ill) { delete ill; }
};

}

#endif /* __MIS_ILLUM_H__ */

// arch-tag: cfd4e0a1-2bc3-4e1a-9206-2aa029c91dfd
