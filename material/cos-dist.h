// cos-dist.h -- Cosine distribution
//
//  Copyright (C) 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_COS_DIST_H
#define SNOGRAY_COS_DIST_H

#include "dist.h"


namespace snogray {


class CosDist : Dist
{
public:

  // Return a sample distributed around the Z-axis according to this
  // distribution, from the uniformly distributed parameters in PARAM.
  //
  Vec sample (const UV &param) const
  {
    return z_normal_symm_vec (sqrt (param.u), param.v);
  }

  // Return a sample distributed around the Z-axis according to this
  // distribution, from the uniformly distributed parameters in PARAM.
  // Also return the PDF of the resulting sample in _PDF.
  //
  Vec sample (const UV &param, float &_pdf) const
  {
    float cos_theta = sqrt (param.u);
    _pdf = pdf (cos_theta);
    return z_normal_symm_vec (cos_theta, param.v);
  }

  // Returns the PDF of a sample in direction DIR.
  //
  float pdf (const Vec &dir) const
  {
    return pdf (dir.z);
  }

  // Returns the pdf of a sample, if COS_THETA is the cosine between it and
  // the normal.
  //
  float pdf (float cos_theta) const
  {
    return cos_theta * INV_PIf;
  }
};


}


#endif /* SNOGRAY_COS_DIST_H */

// arch-tag: d44b5007-a39e-49bb-922d-9c34c4a14da5
