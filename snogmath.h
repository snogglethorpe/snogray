// snogmath.h -- Math operations
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SNOGMATH_H__
#define __SNOGMATH_H__

#include <cmath>
#include <algorithm>  		// for min and max


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


}

#endif // __SNOGMATH_H__


// arch-tag: 03228538-6bb5-4ab4-817b-925702c2c379
