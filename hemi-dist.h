// hemi-dist.h -- Hemisphere distribution
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __HEMI_DIST_H__
#define __HEMI_DIST_H__

#include "dist.h"


namespace snogray {


class HemiDist : Dist
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
    float cos_theta = u;
    _pdf = pdf ();
    return z_normal_symm_vec (cos_theta, v);
  }

  // Returns the pdf of a sample.
  //
  float pdf () const
  {
    return 2 * INV_PIf;
  }
};


}


#endif /* __HEMI_DIST_H__ */

// arch-tag: 06af2c27-b81d-4f17-90d1-07cf0a59f64b
