// xform.h -- 3d affine transformations
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __XFORM_H__
#define __XFORM_H__

#include "coords.h"
#include "vec.h"
#include "uv.h"

#include "xform-base.h"


namespace snogray {


// A 3d affine transformation.
//
template<typename T>
class TXform : public XformBase<T>
{
public:

  // Default is identity transformation
  //
  TXform (T scale = 1) : XformBase<T> (scale) { }

  // A vector/position yields a translation
  //
  template<typename T2>
  TXform (const Tuple3<T2> &tup)
  {
    el (3, 0) = tup.x;
    el (3, 1) = tup.y;
    el (3, 2) = tup.z;
  }

  // A UV value yields a 2d translation
  template<typename T2>
  TXform (const TUV<T2> &uv)
  {
    el (3, 0) = uv.u;
    el (3, 1) = uv.v;
  }

  // Allow easy down-casting from a raw matrix
  //
  template<typename T2>
  TXform (const Matrix4<T2> &m) : XformBase<T> (m) { }
  template<typename T2>
  TXform (const T2 els[4][4]) : XformBase<T> (els) { }
  template<typename T2>
  explicit TXform (const TXform<T2> &xf) : XformBase<T> (xf) { }

  // Return a transform which translates by the given offset.
  //
  static TXform translation (const TVec<T> &offs)
  {
    return TXform (offs);
  }
  static TXform translation (const TUV<T> &offs)
  {
    return TXform (offs);
  }
  static TXform translation (T x, T y, T z = 0)
  {
    return TXform (TVec<T> (x, y, z));
  }

  // Return a transform which scales by a factor of SCALE in all dimensions.
  //
  static TXform scaling (T scale)
  {
    return TXform (scale);
  }

  // Return a transform which scales by factors of S_X, S_Y, and S_Z in the
  // corresponding dimensions.  The Z axis is defaulted for convenient 2d use.
  //
  static TXform scaling (T s_x, T s_y, T s_z = 1)
  {
    TXform xform;
    xform (0, 0) = s_x;
    xform (1, 1) = s_y;
    xform (2, 2) = s_z;
    return xform;
  }

  // Return a rotation transform which rotates around the Y-axis by ANGLE
  // radians.
  //
  static TXform x_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (1, 1) = cos_a;
    xform (2, 1) = -sin_a;
    xform (1, 2) = sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  // Return a rotation transform which rotates around the Y-axis by ANGLE
  // radians.
  //
  static TXform y_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (2, 0) = sin_a;
    xform (0, 2) = -sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  // Return a rotation transform which rotates around the Z-axis by ANGLE
  // radians.
  //
  static TXform z_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (1, 0) = -sin_a;
    xform (0, 1) = sin_a;
    xform (1, 1) = cos_a;
    return xform;
  }

  // Return a rotation transform which rotates around AXIS by ANGLE
  // radians.
  //
  static TXform rotation (const TVec<T> &axis, T angle)
  {
    TXform xform;

    T dx = axis.x, dy = axis.y, dz = axis.z;

    T roll  = -atan2 (dx, sqrt (dy * dy + dz * dz));
    T pitch =  atan2 (dz, dy);

    xform.rotate_x (-pitch);
    xform.rotate_z (-roll);

    xform.rotate_y (angle);

    xform.rotate_z (roll);
    xform.rotate_x (pitch);

    return xform;
  }

  // An alias for z_rotation which is nice for 2d rotations.
  //
  static TXform rotation (T angle)
  {
    return z_rotation (angle);
  }

  // Return a rotation transformation which converts to a coordinate
  // system with (orthonormal) axes X_AXIS, Y_AXIS, and Z_AXIS.
  //
  static TXform
  basis (const TVec<T> &x_axis, const TVec<T> &y_axis, const TVec<T> &z_axis)
  {
    TXform xform;
    xform (0, 0) = x_axis.x; xform (0, 1) = x_axis.y; xform (0, 2) = x_axis.z;
    xform (1, 0) = y_axis.x; xform (1, 1) = y_axis.y; xform (1, 2) = y_axis.z;
    xform (2, 0) = z_axis.x; xform (2, 1) = z_axis.y; xform (2, 2) = z_axis.z;
    return xform;
  }

  // Translate this transformation by the given offset.
  // The Z axis is defaulted for convenient 2d use.
  //
  TXform &translate (T x, T y, T z = 0)
  {
    *this *= translation (x, y, z);
    return *this;
  }
  TXform &translate (const TVec<T> &offs)
  {
    *this *= translation (offs);
    return *this;
  }
  TXform &translate (const TUV<T> &offs)
  {
    *this *= translation (offs);
    return *this;
  }

  // Scale this transform by the scale factor SCALE.
  //
  TXform &scale (T scale)
  {
    *this *= scaling (scale);
    return *this;
  }

  // Scale this transform by the scale factors S_X, S_Y, and S_Z in the
  // corresponding dimensions.  The Z axis is defaulted for convenient 2d
  // use.
  //
  TXform &scale (T s_x, T s_y, T s_z = 1)
  {
    *this *= scaling (s_x, s_y, s_z);
    return *this;
  }

  // Rotate this transform around the given axis, by the given angle
  // (expressed in radians).
  //
  TXform &rotate_x (T angle)
  {
    *this *= x_rotation (angle);
    return *this;
  }
  TXform &rotate_y (T angle)
  {
    *this *= y_rotation (angle);
    return *this;
  }
  TXform &rotate_z (T angle)
  {
    *this *= z_rotation (angle);
    return *this;
  }
  TXform &rotate (TVec<T> axis, T angle)
  {
    *this *= rotation (axis, angle);
    return *this;
  }
  TXform &rotate (T angle)	// alias for rotate_z, for 2d use
  {
    *this *= rotation (angle);
    return *this;
  }

  // Convert from the standard coordinate system to a coordinate system
  // with (orthonormal) axes X_AXIS, Y_AXIS, and Z_AXIS.
  //
  TXform &to_basis (const TVec<T> &x_axis, const TVec<T> &y_axis,
		    const TVec<T> &z_axis)
  {
    *this *= basis (x_axis, y_axis, z_axis);
    return *this;
  }

  // Return the inverse of this transform.
  //
  TXform inverse () const { return XformBase<T>::inverse (); }

  // Destructively invert this transform.
  //
  TXform &invert ()
  {
    XformBase<T>::invert ();
    return *this;
  }

  // Inherit element-access syntax.
  //
  using XformBase<T>::operator();

  // Allow applying a transform to an object using functional notation.
  // The object should support transformation via operator*.
  //
  template<typename OT>
  OT operator() (const OT &obj) const { return obj * *this; }

  // Return VEC transformed by the transpose of this transform.  This is
  // useful for implementing normal transforms (a normal should be
  // transformed by using the transpose of the inverse of the transform,
  // compared to transforming a normal vector).
  //
  TVec<T> transpose_transform (const TVec<T> &vec) const
  {
    return
      TVec<T> (
	(  vec.x * el (0, 0)
	 + vec.y * el (0, 1)
	 + vec.z * el (0, 2)),
	(  vec.x * el (1, 0)
	 + vec.y * el (1, 1)
	 + vec.z * el (1, 2)),
	(  vec.x * el (2, 0)
	 + vec.y * el (2, 1)
	 + vec.z * el (2, 2))
	);
  }

private:

  // Inherit the internal element-access syntax from our base-class.
  //
  using XformBase<T>::el;
};


template<typename T>
std::ostream&
operator<< (std::ostream &os, const TXform<T> &xform)
{
  os << "xform<";

  for (int j = 0; j < 4; j++)
    {
      if (j > 0)
	os << "; ";
      for (int i = 0; i < 4; i++)
	{
	  if (i > 0)
	    os << ", ";
	  os << std::setprecision (5) << lim (xform (i, j));
	}
    }

  os << ">";

  return os;
}


typedef TXform<dist_t>  Xform;
typedef TXform<sdist_t> SXform;


}

#endif // __XFORM_H__


// arch-tag: e168d3bc-8dfe-4a9a-8708-a79db007005e
