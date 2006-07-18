// snogmath.h -- Math operations
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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

namespace Snogray {

// Copy the standard scalar definitions of these operators into our
// namespace, so that they are considered on an equal basis for overloading
// with our overloads.

using std::min;
using std::max;

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
