// vec.h -- Vector datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __VEC_H__
#define __VEC_H__

#include <fstream>
#include <iomanip>

#include "snogmath.h"
#include "tuple3.h"
#include "xform-base.h"


namespace snogray {


template<typename T>
class TVec : public Tuple3<T>
{
public:

  using Tuple3<T>::x;
  using Tuple3<T>::y;
  using Tuple3<T>::z;

  TVec (T _x, T _y, T _z = 0) : Tuple3<T> (_x, _y, _z) { }
  TVec () { }

  // Allow easy down-casting for sharing code
  //
  template<typename T2>
  explicit TVec (const Tuple3<T2> &t) : Tuple3<T> (t) { }

  using Tuple3<T>::operator*=;
  using Tuple3<T>::operator/=;

  bool null () const { return x == 0 && y == 0 && z == 0; }

  TVec operator+ (const TVec &v2) const
  {
    return TVec (x + v2.x, y + v2.y, z + v2.z);
  }
  TVec operator- (const TVec &v2) const
  {
    return TVec (x - v2.x, y - v2.y, z - v2.z);
  }
  TVec operator- () const
  {
    return TVec (-x, -y, -z);
  }

  TVec operator* (T scale) const
  {
    return TVec (x * scale, y * scale, z * scale);
  }
  TVec operator/ (T denom) const
  {
    return operator* (1 / denom);
  }

  void operator+= (const TVec &v2)
  {
    x += v2.x; y += v2.y; z += v2.z;
  }
  void operator-= (const TVec &v2)
  {
    x -= v2.x; y -= v2.y; z -= v2.z;
  }

  TVec operator* (const XformBase<T> &xform) const
  {
    return
      TVec (
	(  x * xform (0, 0)
	 + y * xform (1, 0)
	 + z * xform (2, 0)),
	(  x * xform (0, 1)
	 + y * xform (1, 1)
	 + z * xform (2, 1)),
	(  x * xform (0, 2)
	 + y * xform (1, 2)
	 + z * xform (2, 2))
	);
  }

  const TVec &operator*= (const XformBase<T> &xform)
  {
    TVec temp = *this * xform;
    *this = temp;
    return *this;
  }

  T length_squared () const
  {
    return x * x + y * y + z * z;
  }
  T length () const
  {
    return sqrt (x * x + y * y + z * z);
  }

  TVec unit () const
  {
    T len = length ();
    if (len == 0)
      return TVec (0, 0, 0);
    else
      return operator* (1 / len);
  }

  // Return an arbitrary vector which is perpendicular to this one.
  // The return value is not normalized.
  //
  TVec perpendicular () const
  {
    // If VEC is pointing along the y-axis, just return some other axis.
    // Otherwise, return VEC x (VEC x (0,1,0)).
    //
    if (abs (x) < Eps && abs (z) < Eps)
      return TVec (y, 0, 0);
    else
      return TVec (x * y, -(z * z) - (x * x), y * z);
  }

  T latitude () const { return atan2 (y, sqrt (x * x + z * z)); }
  T colatitude () const { return atan2 (sqrt (x * x + z * z), y); }
  T longitude () const { return atan2 (x, z); }

  // Transform this vector to a coordinate system with (orthonormal)
  // axes X_AXIS, Y_AXIS, and Z_AXIS.
  //
  TVec to_basis (const TVec &x_axis, const TVec &y_axis, const TVec &z_axis)
    const
  {
    return TVec (dot (*this, x_axis), dot (*this, y_axis), dot (*this, z_axis));
  }

  // Transform this vector from a coordinate system with (orthonormal)
  // axes X_AXIS, Y_AXIS, and Z_AXIS to a coordinate system where the
  // axes are (1,0,0), (0,1,0), and (0,0,1).
  //
  TVec from_basis  (const TVec &x_axis, const TVec &y_axis, const TVec &z_axis)
    const
  {
    return TVec (x_axis.x * x + y_axis.x * y + z_axis.x * z,
		 x_axis.y * x + y_axis.y * y + z_axis.y * z,
		 x_axis.z * x + y_axis.z * y + z_axis.z * z);
  }

  // Return the "mirror" of this vector around NORMAL
  //
  TVec mirror (const TVec &normal) const
  {
    // mirror =  2 N (vec . N) - vec

    return  normal * dot (*this, normal) * 2 - *this;
  }

  // Return this vector refracted through a medium transition across a
  // surface with normal NORMAL.  IOR_IN and IOR_OUT are the indices of
  // refraction for the incoming and outgoing media.
  //
  TVec refraction (const TVec &normal, T ior_in, T ior_out) const
  {
    // From:
    //
    //   Heckbert, Paul S., Pat Hanrahan, "Beam Tracing Polygonal Objects,",
    //   _Computer Graphics (SIGGRAPH '84 Proceedings)_, vol. 18, no. 3,
    //   July 1984, pp. 119-127.
    // 

    T ior_ratio = ior_in / ior_out;

    T c1 = -dot (*this, normal);
    T c2_sq = 1 - (ior_ratio * ior_ratio) * (1 - c1 * c1);

    if (c2_sq < -Eps)
      return TVec (0, 0, 0);	// Total internal reflection

    if (c2_sq < 0)
      c2_sq = 0;

    return (*this * ior_ratio) + (normal * (ior_ratio * c1 - sqrt (c2_sq)));
  }
};


template<typename T>
static inline T
dot (const TVec<T> &vec1, const TVec<T> &vec2)
{
  return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

template<typename T>
static inline TVec<T>
cross (const TVec<T> &vec1, const TVec<T> &vec2)
{
  return TVec<T> (vec1.y * vec2.z - vec1.z * vec2.y,
		  vec1.z * vec2.x - vec1.x * vec2.z,
		  vec1.x * vec2.y - vec1.y * vec2.x);
}

template<typename T>
static inline TVec<T>
operator* (T scale, const TVec<T> &vec)
{
  return vec * scale;
}

template<typename T>
static inline TVec<T> abs (const TVec<T> &vec)
{
  return TVec<T> (abs (vec.x), abs (vec.y), abs (vec.z));
}


// Constructor for converting spherical coordinates (relative to the
// y-axis) to a vector.  Note that the first argument is the
// "colatitude", where 0 is straight up along the y axis (as opposed to
// "latitude", where 0 is in the x-z plane).
//
template<typename T>
static inline TVec<T> y_axis_spherical_to_vec (T colat, T lng)
{
  T sin_theta = sin (colat);
  return TVec<T> (sin (lng) * sin_theta, cos (colat), cos (lng) * sin_theta);
}

// Constructor for converting spherical coordinates (relative to the
// y-axis) to a vector, where the colatitude is represented by its
// cosine.
//
template<typename T>
static inline TVec<T> y_axis_cos_spherical_to_vec (T cos_theta, T azimuth)
{
  T sin_theta = sqrt (1 - cos_theta * cos_theta);
  return TVec<T> (sin (azimuth) * sin_theta, cos_theta, cos (azimuth) * sin_theta);
}

// Constructor for converting latitude-longitude coordinates (relative
// to the y-axis) to a vector.
//
template<typename T>
static inline TVec<T> y_axis_latlong_to_vec (T lat, T lng)
{
  T cos_lat = cos (lat);
  return TVec<T> (sin (lng) * cos_lat, sin (lat), cos (lng) * cos_lat);
}


template<typename T>
static std::ostream&
operator<< (std::ostream &os, const TVec<T> &vec)
{
  os << "vec<" << std::setprecision (5) << lim (vec.x)
     << ", " << std::setprecision (5) << lim (vec.y)
     << ", " << std::setprecision (5) << lim (vec.z)
     << ">";
  return os;
}


typedef TVec<sdist_t> SVec;

// XXX Commented out because it causes some weird bloat in generated code,
// even though it's never used!!
//
//typedef TVec<ddist_t> DVec;

typedef TVec<dist_t>  Vec;


}


#endif /* __VEC_H__ */

// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
