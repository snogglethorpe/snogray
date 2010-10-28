// matrix.tcc -- General-purpose matrix type
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_TCC__
#define __MATRIX_TCC__

#include "snogassert.h"


namespace snogray {


template<typename T>
Matrix<T>
Matrix<T>::operator* (T scale) const
{
  Matrix result (_columns, _rows);
  for (unsigned row = 0; row < _rows; row++)
    for (unsigned col = 0; col < _columns; col++)
      result (col, row) = (*this) (col, row) * scale;
  return result;
}


template<typename T>
Matrix<T>
Matrix<T>::operator* (const Matrix &mat) const
{
  //ASSERT (_columns == mat._rows);
  Matrix result (mat._columns, _rows);
  for (unsigned row = 0; row < _rows; row++)
    for (unsigned col = 0; col < mat._columns; col++)
      {
	T sum = 0;
	for (unsigned inner = 0; inner < _columns; inner++)
	  sum += (*this) (inner, row) * mat (col, inner);
	result (col, row) = sum;
      }
  return result;
}


template<typename T>
Matrix<T>
Matrix<T>::transpose () const
{
  Matrix result (_rows, _columns);
  for (unsigned row = 0; row < _rows; row++)
    for (unsigned col = 0; col < _columns; col++)
      result (row, col) = (*this) (col, row);
  return result;
}


}

#endif // __MATRIX_TCC__
