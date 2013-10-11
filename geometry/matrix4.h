// matrix4.h -- 4 x 4 matrices
//
//  Copyright (C) 2005-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MATRIX4_H
#define SNOGRAY_MATRIX4_H


namespace snogray {


template<typename T>
class Matrix4
{
public:

  static const Matrix4 identity;

  // Default constructor returns an identity matrix
  //
  Matrix4 ();

  Matrix4 (T d0, T d1, T d2, T d3);

  template<typename T2>
  Matrix4 (const T2 _els[4][4]);
  template<typename T2>
  Matrix4 (const T2 _els[16]);

  template<typename T2>
  explicit Matrix4 (const Matrix4<T2> &m2);

  T &operator() (unsigned row, unsigned col) { return els[row][col]; }
  const T &operator() (unsigned row, unsigned col) const { return els[row][col]; }

  T &el (unsigned row, unsigned col) { return els[row][col]; }
  const T &el (unsigned row, unsigned col) const { return els[row][col]; }

  Matrix4 operator* (const Matrix4 &xform) const;
  Matrix4 operator* (T scale) const;

  Matrix4 &operator*= (const Matrix4 &mat);
  Matrix4 &operator*= (T scale);

  Matrix4 &invert ();

  Matrix4 transpose () const;

  T det () const;
  Matrix4 adjoint () const;

  Matrix4 inverse () const;

  bool is_identity () const;

private:

  T els[4][4];
};

template<typename T> const Matrix4<T> Matrix4<T>::identity;

}


// Include method definitions
//
#include "matrix4.tcc"


#endif // SNOGRAY_MATRIX4_H

// arch-tag: f013901a-016f-4c68-b102-c5f4c7a5b4a8
