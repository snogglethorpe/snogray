// rand.h -- Random number functions
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RAND_H__
#define __RAND_H__

#include <cstdlib>

namespace Snogray {

inline double random (double limit = 1.0)
{
  return double (rand()) * limit / double (RAND_MAX);
}

inline double random (double min, double limit)
{
  return (double (rand()) * (limit - min) / double (RAND_MAX)) + min;
}

inline float random (float limit = 1.0)
{
  return float (rand()) * limit / float (RAND_MAX);
}

inline float random (float min, float limit)
{
  return (float (rand()) * (limit - min) / float (RAND_MAX)) + min;
}

}

#endif /* __RAND_H__ */

// arch-tag: 70f2c402-3aca-45d2-a09e-44bb847eae81
