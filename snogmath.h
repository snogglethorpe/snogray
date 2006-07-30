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


// Single-precision versions of math constants, to avoid promoting the
// result of a math operation to double.  Note that because these use a
// cast, they cannot be used in constant contexts (e.g., global variable
// initializers).
//
#define _M_F(v) (static_cast<float>(v))
#define M_Ef		(_M_F (M_E))		// e
#define M_LOG2Ef	(_M_F (M_LOG2E))	// log_2 e
#define M_LOG10Ef	(_M_F (M_LOG10E))	// log_10 e
#define M_LN2f		(_M_F (M_LN2))		// log_e 2
#define M_LN10f		(_M_F (M_LN10))		// log_e 10
#define M_PIf		(_M_F (M_PI))		// pi
#define M_PI_2f		(_M_F (M_PI_2))		// pi/2
#define M_PI_4f		(_M_F (M_PI_4))		// pi/4
#define M_1_PIf		(_M_F (M_1_PI))		// 1/pi
#define M_2_PIf		(_M_F (M_2_PI))		// 2/pi
#define M_2_SQRTPIf	(_M_F (M_2_SQRTPI))	// 2/sqrt(pi)
#define M_SQRT2f	(_M_F (M_SQRT2))	// sqrt(2)
#define M_SQRT1_2f	(_M_F (M_SQRT1_2))	// 1/sqrt(2)


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
