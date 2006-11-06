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

// Double-precision random numbers

// Multiplier to turn the integer result from rand(), with range
// [0, MAX_RAND], into a double, range [0, 1).
//
static const double drand_scale = 1 / (double (RAND_MAX) + 1);

inline double drand () { return double (rand()) * drand_scale; }

inline double random (double limit = 1.0)
{
  return drand() * limit;
}

inline double random (double min, double limit)
{
  return (drand() * (limit - min)) + min;
}


// Single-precision random numbers

// Single-precision random numbers are a bit harder, as RAND_MAX
// (on a 32-bit machine) won't fit in a float, and just using
// double for the calculation and converting to float will
// sometimes end up rounding to 1.  So we just fiddle around to
// avoid doing so.

#define RAND_MAX_BITS 		31 // XXX CONFIGURE THIS!!
#define FLOAT_MANTISSA_BITS	24 // XXX ...likewise
#define FRAND_SHIFT		(RAND_MAX_BITS - FLOAT_MANTISSA_BITS)

static const float frand_scale = 1 / float ((RAND_MAX >> FRAND_SHIFT) + 1);

inline float frand () { return float (rand() >> FRAND_SHIFT) * frand_scale; }

inline float random (float limit = 1.0)
{
  return frand() * limit;
}

inline float random (float min, float limit)
{
  return (frand() * (limit - min)) + min;
}


}

#endif /* __RAND_H__ */

// arch-tag: 70f2c402-3aca-45d2-a09e-44bb847eae81
