// ward-dist.h -- Isotropic Ward distribution
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __WARD_DIST_H__
#define __WARD_DIST_H__

#include "dist.h"


namespace Snogray {


class WardDist : Dist
{
public:

  WardDist (float _m)
    : m (_m), inv_m2 (1.f / (m * m))
  { }

  bool operator== (const WardDist &dist) const { return m == dist.m; }

  // Return a sample distributed around the Z-axis according to this
  // distribution, from the uniformly distributed parameters U and V.
  //
  Vec sample (float u, float v) const
  {
    // The original formula is: theta = atan (m * sqrt (-log (1 - u))), but
    // we need cos (theta) to call z_normal_symm_vec, and can use use
    // algebraic rearrangement to eliminate calls to the expensive cos and
    // atan functions.
    //
    float cos_theta = u >= 1 ? 0 : 1 / sqrt (m * m * -log (1 - u) + 1);
    return z_normal_symm_vec (cos_theta, v);
  }

  // Returns the pdf of a sample, if COS_THETA is the cosine between it and
  // the normal.
  //
  float pdf (float cos_theta) const
  {
    if (cos_theta == 0)
      return 0;
    else
      {
	double inv_cos_theta = 1 / cos_theta;
	double inv_cos2_theta = inv_cos_theta * inv_cos_theta;
	double inv_cos3_theta = inv_cos2_theta * inv_cos_theta;
	double tan2_theta = inv_cos2_theta - 1;
	return inv_m2 * M_1_PIf * inv_cos3_theta * exp (-tan2_theta * inv_m2);
      }
  }

  // m:  RMS slope of microfacets -- large m means more spread out
  // reflections.
  //
  float m;

private:  

  // Store 1 / m^2, as that's what the calculations use.
  //
  float inv_m2;
};


}


#endif /* __WARD_DIST_H__ */

// arch-tag: 0bcce561-b31d-4b85-af21-deeeac6abff7
