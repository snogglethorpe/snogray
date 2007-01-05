// envmap-light.v_sz -- Abstract class for envmapured light sources
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ENVMAP_LIGHT_H__
#define __ENVMAP_LIGHT_H__


#include "struct-light.h"
#include "envmap.h"


namespace snogray {


class EnvmapLight : public StructLight
{
public:

  EnvmapLight (const Envmap &envmap);

  // Dump a picture of the generated light regions to a file called
  // FILENAME.  ENVMAP should be the original environment map with which
  // this light was created.
  //
  void dump (const std::string &filename, const Envmap &orig_envmap) const
  {
    StructLight::dump (filename, orig_envmap.light_map ());
  }

  // Generate around NUM samples of this light and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const;

  // For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
  // light, and where light is closer than the sample's previously recorded
  // light distance (or the previous distance is zero), overwrite the
  // sample's light-related fields with information from this light.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const;
};


}

#endif /* __ENVMAP_LIGHT_H__ */

// arch-tag: a6e2dd05-d36c-487b-9ba4-38c4d2f238b0
