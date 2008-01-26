// quadratic-isec.h -- Quadratic surface intersection
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
  // Special-case linear equation to avoid divide-by-zero below.
  //
  if (a == 0)
    {
      if (b == 0)
	return false;

      root0 = root1 = -c / b;
      return true;
    }

  T disc = b * b - 4 * a * c;

  if (disc < 0)
    return false;

  T sqrt_disc = sqrt (disc);
  T t = -(b + (b < 0 ? -sqrt_disc : sqrt_disc)) / 2;

  root0 = t / a;
  root1 = (t == 0) ? root0 : c / t; // if roots are same, t can be 0

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
