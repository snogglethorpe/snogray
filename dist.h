// dist.h -- Common superclass for distribution classes
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __DIST_H__
#define __DIST_H__

#include "snogmath.h"


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
    float sin_theta = sqrt (1 - cos_theta * cos_theta);
    float lng = v * 2 * M_PIf;
    return Vec (sin (lng) * sin_theta, cos (lng) * sin_theta, cos_theta);
  }

};


}


#endif /* __DIST_H__ */

// arch-tag: 0df2598b-4960-43ab-94d1-367835050ed9
