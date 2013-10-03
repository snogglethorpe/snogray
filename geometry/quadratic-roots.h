// quadratic-roots.h -- Quadratic surface intersection
//
//  Copyright (C) 2007, 2008, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_QUADRATIC_ROOTS_H
#define SNOGRAY_QUADRATIC_ROOTS_H

#include "util/snogmath.h"
#include "geometry/ray.h"


namespace snogray {


// Solve the quadratic equation A*X^2 + B*X + C = 0, and return its
// roots in ROOTS.  The number of roots is returned.  If there are
// multiple roots, they will be in sorted order in ROOTS.
//
template<typename T>
inline unsigned
quadratic_roots (T a, T b, T c, T roots[2])
{
  // Special-case linear equation to avoid divide-by-zero below.
  //
  if (a == 0)
    {
      if (b == 0)
	return 0;

      roots[0] = -c / b;
      return 1;
    }

  T disc = b * b - 4 * a * c;

  if (disc < 0)
    return 0;

  T sqrt_disc = sqrt (disc);
  T t = -(b + (b < 0 ? -sqrt_disc : sqrt_disc)) / 2;

  roots[0] = t / a;

  if (t == 0)
    return 1;			// only one root
  else
    {
      roots[1] = c / t;

      if (roots[0] > roots[1])
	{
	  T tmp = roots[0];
	  roots[0] = roots[1];
	  roots[1] = tmp;
	}

      return 2;
    }
}


}


#endif // SNOGRAY_QUADRATIC_ROOTS_H

// arch-tag: ea3d793e-aa16-424e-ad19-0e2f0edfa353
