// vec.h -- Vector datatype
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_VEC_H
#define SNOGRAY_VEC_H

#include "tuple3.h"
#include "xform-base.h"
#include "compiler.h"


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

  // Return this vector transformed by XFORM.
  //
  TVec transformed (const XformBase<T> &xform) const
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

  // Transform this vector by XFORM.
  //
  void transform (const XformBase<T> &xform)
  {
    *this = xform (*this);
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
    return operator* (1 / len);
  }

  // Return an arbitrary vector which is perpendicular to this one.
  // The return value is not normalized.
  //
  TVec perpendicular () const
  {
    // This is [x,y,z] x [-z,x,y]; to avoid degenerate behavior in the
    // case where x = z = -y, we flip z's sign if x == z.
    //
    T nz = unlikely (x == z) ? z : -z;
    return TVec (y*y + x*nz, -nz*nz - x*y, x*x - y*nz);
  }

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


typedef TVec<sdist_t> SVec;

// XXX Commented out because it causes some weird bloat in generated code,
// even though it's never used!!
//
//typedef TVec<ddist_t> DVec;

typedef TVec<dist_t>  Vec;


}


#endif /* SNOGRAY_VEC_H */

// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
