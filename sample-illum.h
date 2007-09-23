// sample-illum.h -- Sample-based illuminator
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE_ILLUM_H__
#define __SAMPLE_ILLUM_H__

#include <vector>

#include "illum-sample.h"
#include "illum.h"


namespace snogray {

class Intersect;
class Trace;


class SampleIllum : public Illum
{
public:
  
  struct LightParams
  {
    LightParams ()
      : num_samples (0), num_real_samples (0),
	weight (0), is_point_light (false)
    { }

    // This is the "logical" number of samples, including samples
    // suppressed because they have a value of zero.
    //
    unsigned num_samples;

    // This is the physical number of samples actually added.
    //
    unsigned num_real_samples;

    // Weight applied to light.  The weights of all lights add to one.
    //
    float weight;

    bool is_point_light;
  };

  SampleIllum (const Trace &trace);

  // Illuminate the intersection ISEC.
  //
  virtual Color illum (const Intersect &isec);

  // Generate samples for estimating the illumination at ISEC.  The
  // samples should be appended to SAMPLES.  NUM_BRDF_SAMPLES is the
  // number of brdf samples to use, and LIGHT_PARAMS is an array of
  // sampling parameters for each light (which may be modified).
  //
  virtual void gen_samples (const Intersect &isec,
			    unsigned num_brdf_samples,
			    std::vector<LightParams> &light_params,
			    IllumSampleVec &samples)
    = 0;

  // Return the illumination from the samples in SAMPLES.
  //
  Color eval_samples (const Intersect &isec, IllumSampleVec &samples);

  const std::vector<const Light *> &lights;
  unsigned num_lights;

//private:

  void distribute_light_samples (unsigned num,
				 std::vector<LightParams> &light_params);

  // Most of the following are only used locally during illumination, but
  // are kept around to avoid the overhead of repeated allocation/deallocation.

  // Vector of samples.
  //
  IllumSampleVec samples;

  // Sampling parameters for each light, indexed by light.
  //
  std::vector<LightParams> light_params;

  unsigned num_area_lights, num_point_lights;
};


}

#endif /* __SAMPLE_ILLUM_H__ */

// arch-tag: 8f0e4b8d-3a62-497d-8453-6dda9717342c
