// lsamples.cc -- Sampling of light sources irradiating a surface point
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <algorithm>
#include <functional>

#include "intersect.h"
#include "brdf.h"

#include "lsamples.h"

using namespace Snogray;

// Comparison class for sorting the sample indices by sample value
//
struct indexed_sample_val_gtr
{
  indexed_sample_val_gtr (const SampleRayVec &_samples)
    : samples (_samples)
  { }
  bool operator() (const unsigned &idx1, const unsigned &idx2) const
  {
    return samples[idx1].val > samples[idx2].val;
  }

  const SampleRayVec &samples;
};

void
LightSamples::generate (const Intersect &isec,
			const std::vector<const Light *> lights)
{
  // Fill the sample vector
  //
  // XXX this currently generates light samples and then filters them
  // through the BRDF; we should later change this to choose the
  // direction (light-BRDF or BRDF-light) separately for each light;
  // ideally whose importance function has higher-frequency components
  // should come first.

  samples.clear ();

  for (std::vector<const Light *>::const_iterator li = lights.begin();
       li != lights.end(); li++)
    (*li)->gen_samples (isec, samples);
 
  isec.brdf.filter_samples (isec, samples, samples.begin(), samples.end());

  // Generate indices into the sample vector, and sort them.

  unsigned nsamples = samples.size ();
  sample_indices.clear ();
  sample_indices.reserve (nsamples);

  for (unsigned i = 0; i < nsamples; i++)
    sample_indices.push_back (i);

  std::sort (sample_indices.begin (), sample_indices.end (),
	     indexed_sample_val_gtr (samples));
}

// arch-tag: 44a3b598-bd09-4dab-927a-682ed5ab60c4
