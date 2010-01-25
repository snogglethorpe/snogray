// mis-illum.h -- Direct illumination using multiple importance sampling
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIS_ILLUM_H__
#define __MIS_ILLUM_H__

#include "direct-illum.h"


namespace snogray {


class MisIllum : public DirectIllum
{
public:

  MisIllum (const Scene &scene)
    : DirectIllum (scene,
		   USES_BRDF_SAMPLES | USES_DIRECT_INFO | USES_LIGHT_INFO)
  { }

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
		    const IllumMgr &illum_mgr)
    const;
};


}

#endif /* __MIS_ILLUM_H__ */


// arch-tag: cfd4e0a1-2bc3-4e1a-9206-2aa029c91dfd
