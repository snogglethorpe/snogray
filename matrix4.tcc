// matrix4.tcc -- 4 x 4 matrices
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

#ifndef __MATRIX4_TCC__
#define __MATRIX4_TCC__

#include "config.h"
#include "coords.h"


namespace snogray {


// Default constructor returns an identity matrix
//
template<typename T>
Matrix4<T>::Matrix4 ()
{
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      el (col, row) = (col == row) ? T(1) : T(0);
}

template<typename T>
Matrix4<T>::Matrix4 (T d0, T d1, T d2, T d3)
{
  els[0][0] = d0; els[0][1] = 0; els[0][2] = 0; els[0][3] = 0;
  els[1][0] = 0; els[1][1] = d1; els[1][2] = 0; els[1][3] = 0;
  els[2][0] = 0; els[2][1] = 0; els[2][2] = d2; els[2][3] = 0;
  els[3][0] = 0; els[3][1] = 0; els[3][2] = 0; els[3][3] = d3;
}

template<typename T> template<typename T2>
Matrix4<T>::Matrix4 (const T2 _els[4][4])
{
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      els[row][col] = _els[row][col];
}

template<typename T> template<typename T2>
Matrix4<T>::Matrix4 (const T2 _els[16])
{
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      el (col, row) = _els[row * 4 + col];
}

template<typename T> template<typename T2>
Matrix4<T>::Matrix4 (const Matrix4<T2> &m2)
{
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      els[row][col] = m2 (col, row);
}

template<typename T>
Matrix4<T>
Matrix4<T>::operator* (const Matrix4<T> &xform) const
{
  Matrix4<T> result;
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      result (col, row)
	= el (0, row) * xform (col, 0)
	+ el (1, row) * xform (col, 1)
	+ el (2, row) * xform (col, 2)
	+ el (3, row) * xform (col, 3);
  return result;
}

template<typename T>
Matrix4<T>
Matrix4<T>::operator* (T scale) const
{
  Matrix4<T> result;
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      result (col, row) = el (col, row) * scale;
  return result;
}

template<typename T>
Matrix4<T> &
Matrix4<T>::operator*= (const Matrix4<T> &mat)
{
  *this = *this * mat;
  return *this;
}
template<typename T>
Matrix4<T> &
Matrix4<T>::operator*= (T scale)
{
  *this = *this * scale;
  return *this;
}

template<typename T>
Matrix4<T> &
Matrix4<T>::invert ()
{
  *this = this->inverse ();
  return *this;
}

template<typename T>
Matrix4<T>
Matrix4<T>::transpose () const
{
  Matrix4 result;
  result (0, 0) = el (0, 0); result (0, 1) = el (1, 0);
  result (0, 2) = el (2, 0); result (0, 3) = el (3, 0);
  result (1, 0) = el (0, 1); result (1, 1) = el (1, 1);
  result (1, 2) = el (2, 1); result (1, 3) = el (3, 1);
  result (2, 0) = el (0, 2); result (2, 1) = el (1, 2);
  result (2, 2) = el (2, 2); result (2, 3) = el (3, 2);
  result (3, 0) = el (0, 3); result (3, 1) = el (1, 3);
  result (3, 2) = el (2, 3); result (3, 3) = el (3, 3);
  return result;
}

template<typename T>
T
Matrix4<T>::det () const
{
  return
    el(0,3) * el(1,2) * el(2,1) * el(3,0)
    - el(0,2) * el(1,3) * el(2,1) * el(3,0)
    - el(0,3) * el(1,1) * el(2,2) * el(3,0)
    + el(0,1) * el(1,3) * el(2,2) * el(3,0)
    + el(0,2) * el(1,1) * el(2,3) * el(3,0)
    - el(0,1) * el(1,2) * el(2,3) * el(3,0)
    - el(0,3) * el(1,2) * el(2,0) * el(3,1)
    + el(0,2) * el(1,3) * el(2,0) * el(3,1)
    + el(0,3) * el(1,0) * el(2,2) * el(3,1)
    - el(0,0) * el(1,3) * el(2,2) * el(3,1)
    - el(0,2) * el(1,0) * el(2,3) * el(3,1)
    + el(0,0) * el(1,2) * el(2,3) * el(3,1)
    + el(0,3) * el(1,1) * el(2,0) * el(3,2)
    - el(0,1) * el(1,3) * el(2,0) * el(3,2)
    - el(0,3) * el(1,0) * el(2,1) * el(3,2)
    + el(0,0) * el(1,3) * el(2,1) * el(3,2)
    + el(0,1) * el(1,0) * el(2,3) * el(3,2)
    - el(0,0) * el(1,1) * el(2,3) * el(3,2)
    - el(0,2) * el(1,1) * el(2,0) * el(3,3)
    + el(0,1) * el(1,2) * el(2,0) * el(3,3)
    + el(0,2) * el(1,0) * el(2,1) * el(3,3)
    - el(0,0) * el(1,2) * el(2,1) * el(3,3)
    - el(0,1) * el(1,0) * el(2,2) * el(3,3)
    + el(0,0) * el(1,1) * el(2,2) * el(3,3);
}

template<typename T>
Matrix4<T>
Matrix4<T>::adjoint () const
{
  Matrix4<T> result;

  result(0,0) =
    el(1,2)*el(2,3)*el(3,1) - el(1,3)*el(2,2)*el(3,1)
    + el(1,3)*el(2,1)*el(3,2) - el(1,1)*el(2,3)*el(3,2)
    - el(1,2)*el(2,1)*el(3,3) + el(1,1)*el(2,2)*el(3,3);
  result(0,1) =
    el(0,3)*el(2,2)*el(3,1) - el(0,2)*el(2,3)*el(3,1)
    - el(0,3)*el(2,1)*el(3,2) + el(0,1)*el(2,3)*el(3,2)
    + el(0,2)*el(2,1)*el(3,3) - el(0,1)*el(2,2)*el(3,3);
  result(0,2) =
    el(0,2)*el(1,3)*el(3,1) - el(0,3)*el(1,2)*el(3,1)
    + el(0,3)*el(1,1)*el(3,2) - el(0,1)*el(1,3)*el(3,2)
    - el(0,2)*el(1,1)*el(3,3) + el(0,1)*el(1,2)*el(3,3);
  result(0,3) =
    el(0,3)*el(1,2)*el(2,1) - el(0,2)*el(1,3)*el(2,1)
    - el(0,3)*el(1,1)*el(2,2) + el(0,1)*el(1,3)*el(2,2)
    + el(0,2)*el(1,1)*el(2,3) - el(0,1)*el(1,2)*el(2,3);
  result(1,0) =
    el(1,3)*el(2,2)*el(3,0) - el(1,2)*el(2,3)*el(3,0)
    - el(1,3)*el(2,0)*el(3,2) + el(1,0)*el(2,3)*el(3,2)
    + el(1,2)*el(2,0)*el(3,3) - el(1,0)*el(2,2)*el(3,3);
  result(1,1) =
    el(0,2)*el(2,3)*el(3,0) - el(0,3)*el(2,2)*el(3,0)
    + el(0,3)*el(2,0)*el(3,2) - el(0,0)*el(2,3)*el(3,2)
    - el(0,2)*el(2,0)*el(3,3) + el(0,0)*el(2,2)*el(3,3);
  result(1,2) =
    el(0,3)*el(1,2)*el(3,0) - el(0,2)*el(1,3)*el(3,0)
    - el(0,3)*el(1,0)*el(3,2) + el(0,0)*el(1,3)*el(3,2)
    + el(0,2)*el(1,0)*el(3,3) - el(0,0)*el(1,2)*el(3,3);
  result(1,3) =
    el(0,2)*el(1,3)*el(2,0) - el(0,3)*el(1,2)*el(2,0)
    + el(0,3)*el(1,0)*el(2,2) - el(0,0)*el(1,3)*el(2,2)
    - el(0,2)*el(1,0)*el(2,3) + el(0,0)*el(1,2)*el(2,3);
  result(2,0) =
    el(1,1)*el(2,3)*el(3,0) - el(1,3)*el(2,1)*el(3,0)
    + el(1,3)*el(2,0)*el(3,1) - el(1,0)*el(2,3)*el(3,1)
    - el(1,1)*el(2,0)*el(3,3) + el(1,0)*el(2,1)*el(3,3);
  result(2,1) =
    el(0,3)*el(2,1)*el(3,0) - el(0,1)*el(2,3)*el(3,0)
    - el(0,3)*el(2,0)*el(3,1) + el(0,0)*el(2,3)*el(3,1)
    + el(0,1)*el(2,0)*el(3,3) - el(0,0)*el(2,1)*el(3,3);
  result(2,2) =
    el(0,1)*el(1,3)*el(3,0) - el(0,3)*el(1,1)*el(3,0)
    + el(0,3)*el(1,0)*el(3,1) - el(0,0)*el(1,3)*el(3,1)
    - el(0,1)*el(1,0)*el(3,3) + el(0,0)*el(1,1)*el(3,3);
  result(2,3) =
    el(0,3)*el(1,1)*el(2,0) - el(0,1)*el(1,3)*el(2,0)
    - el(0,3)*el(1,0)*el(2,1) + el(0,0)*el(1,3)*el(2,1)
    + el(0,1)*el(1,0)*el(2,3) - el(0,0)*el(1,1)*el(2,3);
  result(3,0) =
    el(1,2)*el(2,1)*el(3,0) - el(1,1)*el(2,2)*el(3,0)
    - el(1,2)*el(2,0)*el(3,1) + el(1,0)*el(2,2)*el(3,1)
    + el(1,1)*el(2,0)*el(3,2) - el(1,0)*el(2,1)*el(3,2);
  result(3,1) =
    el(0,1)*el(2,2)*el(3,0) - el(0,2)*el(2,1)*el(3,0)
    + el(0,2)*el(2,0)*el(3,1) - el(0,0)*el(2,2)*el(3,1)
    - el(0,1)*el(2,0)*el(3,2) + el(0,0)*el(2,1)*el(3,2);
  result(3,2) =
    el(0,2)*el(1,1)*el(3,0) - el(0,1)*el(1,2)*el(3,0)
    - el(0,2)*el(1,0)*el(3,1) + el(0,0)*el(1,2)*el(3,1)
    + el(0,1)*el(1,0)*el(3,2) - el(0,0)*el(1,1)*el(3,2);
  result(3,3) =
    el(0,1)*el(1,2)*el(2,0) - el(0,2)*el(1,1)*el(2,0)
    + el(0,2)*el(1,0)*el(2,1) - el(0,0)*el(1,2)*el(2,1)
    - el(0,1)*el(1,0)*el(2,2) + el(0,0)*el(1,1)*el(2,2);

  return result;
}

template<typename T>
Matrix4<T>
Matrix4<T>::inverse () const
{
  return adjoint () * (1 / det ());
}

template<typename T>
bool
Matrix4<T>::is_identity () const
{
  for (unsigned row = 0; row < 4; row++)
    for (unsigned col = 0; col < 4; col++)
      {
	T goal = (row == col) ? T(1) : T(0);
	T delta = abs (el (col, row) - goal);
	if (delta > 0.000001)
	  return false;
      }
  return true;
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "matrix4.cc".
// 
#if HAVE_EXTERN_TEMPLATE
EXTERN_TEMPLATE_EXTENSION extern template class Matrix4<dist_t>;
#endif


}

#endif // __MATRIX4_TCC__
