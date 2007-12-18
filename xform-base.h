// xform-base.h -- Base-class for transformation matrices
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __XFORM_BASE_H__
#define __XFORM_BASE_H__

#include "matrix4.h"


namespace snogray {


// XformBase basically serves as a "raw" base-class for the more Deluxe
// TXform class.  Typical uses never use this class.
//
// It is used to avoid include-file recursion problems caused by various
// primitive types which implement their own transformation methods, but
// are in turn referenced in Xform methods.
//
// Such classes should define their transformation methods in terms of
// XformBase instead of TXform.
//
template<typename T>
class XformBase : public Matrix4<T>
{
public:

  // Default is identity transformation
  //
  XformBase (T scale = 1) : Matrix4<T> (scale, scale, scale, 1) { }

  // Allow easy down-casting from a raw matrix
  //
  template<typename T2>
  XformBase (const Matrix4<T2> &m) : Matrix4<T> (m) { }
  template<typename T2>
  XformBase (const T2 els[4][4]) : Matrix4<T> (els) { }
  template<typename T2>
  explicit XformBase (const XformBase<T2> &xf) : Matrix4<T> (xf) { }

  // Return the inverse of this transform.
  //
  XformBase inverse () const { return Matrix4<T>::inverse (); }

  // Destructively invert this transform.
  //
  XformBase &invert ()
  {
    Matrix4<T>::invert ();
    return *this;
  }

  // Return true if this transform reverses the "handedness" of a
  // coordinate system.
  //
  bool reverses_handedness () const { return Matrix4<T>::det() < 0; }

protected:

  // Inherit the internal element-access syntax from our base-class.
  //
  using Matrix4<T>::el;
};


}


#endif /* __XFORM_BASE_H__ */
