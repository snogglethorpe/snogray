// cos-dist.h -- Cosine distribution
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COS_DIST_H__
#define __COS_DIST_H__

#include "dist.h"


namespace snogray {


class CosDist : Dist
{
public:

  // Return a sample distributed around the Z-axis according to this
  // distribution, from the uniformly distributed parameters U and V.
  //
  Vec sample (float u, float v) const
  {
    return z_normal_symm_vec (sqrt (u), v);
  }

  // Return a sample distributed around the Z-axis according to this
  // distribution, from the uniformly distributed parameters U and V.
  // Also return the PDF of the resulting sample.
  //
  Vec sample (float u, float v, float &_pdf) const
  {
    float cos_theta = sqrt (u);
    _pdf = pdf (cos_theta);
    return z_normal_symm_vec (cos_theta, v);
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


#endif /* __COS_DIST_H__ */

// arch-tag: d44b5007-a39e-49bb-922d-9c34c4a14da5
