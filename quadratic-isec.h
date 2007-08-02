// quadratic-isec.h -- Quadratic surface intersection
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __QUADRATIC_ISEC_H__
#define __QUADRATIC_ISEC_H__

#include "snogmath.h"
#include "ray.h"


namespace snogray {


template<typename T>
inline bool
quadratic_isec (T a, T b, T c, T &root0, T &root1)
{
  T disc = b * b - 4 * a * c;

  if (disc < 0)
    return false;

  T sqrt_disc = sqrt (disc);
  T t = -(b + (b < 0 ? -sqrt_disc : sqrt_disc)) / 2;

  root0 = t / a;
  root1 = c / t;

  if (root0 > root1)
    {
      T tmp = root0;
      root0 = root1;
      root1 = tmp;
    }

  return true;
}


}


#endif // __QUADRATIC_ISEC_H__

// arch-tag: ea3d793e-aa16-424e-ad19-0e2f0edfa353
