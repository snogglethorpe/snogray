// illum.h -- Illuminator
//
//  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ILLUM_H__
#define __ILLUM_H__

#include "color.h"
#include "illum-sample.h"


namespace snogray {


class Intersect;
class IllumMgr;


class Illum
{
public:
  
  // Flags that describe what information this illuminator uses.
  //
  // Primarily these control details of the BRDF samples passed into the
  // Illum::lo method.
  //
  enum Uses {
    USES_BRDF_SAMPLES 	= 0x1, // uses BRDF samples in some way
    USES_INTERSECT_INFO = 0x2, // uses "isec_info" field in BRDF samples
    USES_DIRECT_INFO    = 0x4, // uses the BRDF sample IllumSample::DIRECT flag
    USES_LIGHT_INFO     = 0x8, // uses light-related info in BRDF samples
  };


  Illum (unsigned _uses = 0) : uses (_uses) { }
  virtual ~Illum () { }

  // Return outgoing radiance for this illuminator.  The BRDF samples
  // between BRDF_SAMPLES_BEG and BRDF_SAMPLES_END are matched to this
  // illuminator.  NUM_BRDF_SAMPLES is the total number of non-specular
  // BRDF samples generated (even those not passed to this illuminator).
  //
  // ILLUM_MGR can be used for recursively calculating illumination.
  //
  virtual Color Lo (const Intersect &isec,
		    const IllumSampleVec::iterator &brdf_samples_beg,
		    const IllumSampleVec::iterator &brdf_samples_end,
		    unsigned num_brdf_samples,
		    const IllumMgr &illum_mgr, float complexity)
    const = 0;

  // Flags that describe what information this illuminator uses.
  //
  unsigned uses;
};


}

#endif // __ILLUM_H__
