// recurs-illum.h -- Illuminator for specular reflection / refraction
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RECURS_ILLUM_H__
#define __RECURS_ILLUM_H__

#include "illum.h"


namespace snogray {


class RecursIllum : public Illum
{
public:

  RecursIllum (const Scene &)
    : Illum (USES_BRDF_SAMPLES | USES_INTERSECT_INFO),
      rr_depth (5)
  { }

  // Return outgoing radiance for this illuminator.  The BRDF samples
  // between BRDF_SAMPLES_BEG and BRDF_SAMPLES_END are matched to this
  // illuminator.  NUM_BRDF_SAMPLES is the total number of non-specular
  // BRDF samples generated (even those not passed to this illuminator).
  //
  // ILLUM_MGR can be used for recursively calculating illumination.
  //
  virtual Color lo (const Intersect &isec,
		    const IllumSampleVec::iterator &brdf_samples_beg,
		    const IllumSampleVec::iterator &brdf_samples_end,
		    unsigned num_brdf_samples,
		    const IllumMgr &illum_mgr)
    const;

  unsigned rr_depth;
};


}

#endif /* __RECURS_ILLUM_H__ */
