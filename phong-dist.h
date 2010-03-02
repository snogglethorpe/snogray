// phong-dist.h -- Phong distribution
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHONG_DIST_H__
#define __PHONG_DIST_H__

#include "dist.h"


namespace snogray {


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
  Vec sample (const UV &param) const
  {
    return z_normal_symm_vec (pow (param.u, inv_exp_plus_1), param.v);
  }

  // Returns a sample distributed around the y axis accordign to this
  // distribution, and also the corresponding pdf.
  //
  Vec sample (const UV &param, float &_pdf) const
  {
    float cos_theta = pow (param.u, inv_exp_plus_1);
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
    return (exp + 1) * INV_PIf * 0.5f * pow (cos_theta, exp);
  }

private:  

  float exp, inv_exp_plus_1;
};


}


#endif /* __PHONG_DIST_H__ */

// arch-tag: 9a57ed96-ec71-4d7e-bee0-f2c918277995
