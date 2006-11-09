// phong-dist.h -- Phong distribution
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHONG_DIST_H__
#define __PHONG_DIST_H__

#include "dist.h"


namespace Snogray {


class PhongDist : Dist
{
public:

  PhongDist (float exponent)
    : exp (exponent), inv_exp_plus_1 (1 / (exp + 1))
  { }

  bool operator== (const PhongDist &dist) const { return exp == dist.exp; }

  // Returns a sample distributed around the y axis accordign to this
  // distribution.
  //
  Vec sample (float u, float v)
  {
    return z_normal_symm_vec (pow (u, inv_exp_plus_1), v);
  }

  // Returns a sample distributed around the y axis accordign to this
  // distribution, and also the corresponding pdf.
  //
  Vec sample (float u, float v, float &_pdf)
  {
    float cos_theta = pow (u, inv_exp_plus_1);
    _pdf = pdf (cos_theta);
    return z_normal_symm_vec (cos_theta, v);
  }

  // Returns the pdf of a sample, if COS_THETA is the cosine between it and
  // the normal.
  //
  float pdf (float cos_theta)
  {
    return (exp + 1) * M_1_PIf * 0.5f * pow (cos_theta, exp);
  }

private:  

  float exp, inv_exp_plus_1;
};


}


#endif /* __PHONG_DIST_H__ */

// arch-tag: 9a57ed96-ec71-4d7e-bee0-f2c918277995
