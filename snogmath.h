// snogmath.h -- Math operations
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SNOGMATH_H__
#define __SNOGMATH_H__

#include <cmath>
#include <algorithm>  		// for min and max

#include "compiler.h"


namespace snogray {


// Double-precision math constants.
//
static const double	PI     = 3.14159265358979323846;	// pi
static const double	INV_PI = 0.31830988618379067154;	// 1/pi
static const double	E      = 2.7182818284590452354;		// e

// Single-precision math constants.
//
static const float	PIf     = 3.14159265358979323846f;	// pi
static const float	INV_PIf = 0.31830988618379067154f;	// 1/pi
static const float	Ef      = 2.7182818284590452354f;	// e


// Provide our own definitions of min/max, as the standard defs are very slow.

inline float min (float x, float y) { return x < y ? x : y; }
inline double min (double x, double y) { return x < y ? x : y; }
inline int min (int x, int y) { return x < y ? x : y; }
inline unsigned min (unsigned x, unsigned y) { return x < y ? x : y; }

inline float max (float x, float y) { return x > y ? x : y; }
inline double max (double x, double y) { return x > y ? x : y; }
inline int max (int x, int y) { return x > y ? x : y; }
inline unsigned max (unsigned x, unsigned y) { return x > y ? x : y; }


// Clamp VAL to remain between MINV and MAXV (inclusive).
//
template<typename T>
inline T clamp (T val, T minv, T maxv)
{
  val = likely (val < maxv) ? val : maxv;
  return likely (val > minv) ? val : minv;
}


// Clamp VAL to the range 0-1.
//
template<typename T>
inline T clamp01 (T val)
{
  return clamp (val, T (0), T (1));
}


// Copy the standard scalar definitions of these operators into our
// namespace, so that they are considered on an equal basis for overloading
// with our overloads.

using std::abs;
using std::div;
using std::acos;
using std::asin;
using std::atan;
using std::atan2;
using std::ceil;
using std::cos;
using std::cosh;
using std::exp;
using std::fabs;
using std::floor;
using std::fmod;
using std::frexp;
using std::ldexp;
using std::log;
using std::log10;
using std::modf;
using std::pow;
using std::sin;
using std::sinh;
using std::sqrt;
using std::tan;
using std::tanh;

using ::trunc;			// not defined in <cmath>?!


}

#endif // __SNOGMATH_H__


// arch-tag: 03228538-6bb5-4ab4-817b-925702c2c379
