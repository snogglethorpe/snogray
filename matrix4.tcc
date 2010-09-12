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
  els[0][0] = 1; els[0][1] = 0; els[0][2] = 0; els[0][3] = 0;
  els[1][0] = 0; els[1][1] = 1; els[1][2] = 0; els[1][3] = 0;
  els[2][0] = 0; els[2][1] = 0; els[2][2] = 1; els[2][3] = 0;
  els[3][0] = 0; els[3][1] = 0; els[3][2] = 0; els[3][3] = 1;
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
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; i < 4; i++)
      els[i][j] = _els[i][j];
}

template<typename T> template<typename T2>
Matrix4<T>::Matrix4 (const Matrix4<T2> &m2)
{
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 4; j++)
      els[i][j] = m2 (i, j);
}

template<typename T>
Matrix4<T>
Matrix4<T>::operator* (const Matrix4<T> &xform) const
{
  Matrix4<T> result;
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 4; j++)
      result(i, j)
	= el (i, 0) * xform (0, j)
	+ el (i, 1) * xform (1, j)
	+ el (i, 2) * xform (2, j)
	+ el (i, 3) * xform (3, j);
  return result;
}

template<typename T>
Matrix4<T>
Matrix4<T>::operator* (T scale) const
{
  Matrix4<T> result;
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 4; j++)
      result(i, j) = el (i, j) * scale;
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
  for (unsigned j = 0; j < 4; j++)
    for (unsigned i = 0; i < 4; i++)
      {
	T goal = (i == j) ? T(1) : T(0);
	T delta = abs (els[i][j] - goal);
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
extern template class Matrix4<dist_t>;
#endif


}

#endif // __MATRIX4_TCC__
