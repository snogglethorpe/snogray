// dist.h -- Common superclass for distribution classes
//
//  Copyright (C) 2006, 2007, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_DIST_H
#define SNOGRAY_DIST_H

#include "util/snogmath.h"
#include "geometry/vec.h"
#include "geometry/spherical-coords.h"


namespace snogray {


class Dist
{
public:

  // Return a vector in a distribution symmetric around the Z axis.
  // COS_THETA is the cosine of the angle between the Z axis and the
  // vector, and V is the rotation of that vector around the Z axis.
  // V's range is 0-1 (to match common usage), so it is multiplied by
  // 2*PI to get the real rotation angle.
  //
  Vec z_normal_symm_vec (float cos_theta, float v) const
  {
    return z_axis_cos_spherical_to_vec (cos_theta, v * 2 * PIf);
  }

};


}


#endif /* SNOGRAY_DIST_H */

// arch-tag: 0df2598b-4960-43ab-94d1-367835050ed9
