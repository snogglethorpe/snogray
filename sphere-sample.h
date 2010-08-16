// sphere-sample.h -- Sample a sphere
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

#ifndef __SPHERE_SAMPLE_H__
#define __SPHERE_SAMPLE_H__

#include "cone-sample.h"


namespace snogray {


// Return a sample vector uniformly distributed over a unit-sphere.
// PARAM is the sample parameter.
//
static inline Vec
sphere_sample (const UV &param)
{
  // Sampling an entire sphere is a special case of sampling a
  // degenerate "cone", where the cone's angle is 2*PI (a rather wacky
  // cone, but it works).
  //
  // [Note that recent versions of gcc can completely eliminate the
  // resulting PIf * INV_PIf calculation during constant-folding, so
  // there's no need to special-case a sphere.]
  //
  return cone_sample (2*PIf, param);
}


}

#endif // __SPHERE_SAMPLE_H__
