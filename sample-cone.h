// sample-cone.h -- Sample a cone
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE_CONE_H__
#define __SAMPLE_CONE_H__

#include "snogmath.h"
#include "vec.h"
#include "uv.h"


namespace snogray {


// Return a sample vector uniformly distributed over a cone centered
// around the z-axis, whose surface has an angle of HALF_ANGLE from
// the z-axis.  PARAM is the sample parameter.
//
static inline Vec
sample_cone (float half_angle, const UV &param)
{
  // Choose a slice on the upper part of a unit cylinder.  The total
  // height of the cylinder is 2 (from -1 to 1), and the height of the
  // sample area corresponds to HALF_ANGLE (so that if HALF_ANGLE is
  // 2*PI, the entire cylinder surface, from z -1 to 1, will be
  // sampled).
  //
  // Then project the chosen slice onto a unit sphere.  Z is the
  // z-coordinate of the slice, and R is its radius.
  //
  float z = 1 - param.u * half_angle * INV_PIf;
  float r = sqrt (max (1 - z * z, 0.f));

  // Now choose a point around the edge of the radius R disk; X and Y
  // will be the x/y-coordinates of that point.
  //
  float phi = param.v * 2 * PIf;
  float x = r * cos (phi);
  float y = r * sin (phi);

  // Our final vector points from the origin to the chosen point on
  // the edge of the disk.
  //
  return Vec (x, y, z);
}


}

#endif // __SAMPLE_CONE_H__
