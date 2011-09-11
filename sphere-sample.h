// sphere-sample.h -- Sample a sphere
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SPHERE_SAMPLE_H
#define SNOGRAY_SPHERE_SAMPLE_H

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
  return cone_sample (-1, param);
}
 
// Return a parameter for which sample_sphere would return DIR.
//
static inline UV
sphere_sample_inverse (const Vec &dir)
{
  return cone_sample_inverse (-1, dir);
}

// Return the PDF for a sphere sample.
//
static inline float
sphere_sample_pdf ()
{
  return cone_sample_pdf (-1);
}


}

#endif // SNOGRAY_SPHERE_SAMPLE_H
