// matrix.h -- General-purpose matrix type
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

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>


namespace snogray {


// Matrix is a general-purpose matrix type, not really optimized for
// speed, but more flexible than special-purpose types like Matrix4.
//
template<typename T>
class Matrix
{
public:

  // Make an empty matrix, which can later be assigned to from a
  // non-empty matrix.
  //
  Matrix () : _columns (0), _rows (0) { }

  // Make a COLUMNS x ROWS sized matrix with uninitialized ata.
  //
  Matrix (unsigned columns, unsigned rows)
    : _columns (columns), _rows (rows),
      _data (columns * rows)	// default-initialized!
  { }

  // Make a COLUMNS x ROWS sized matrix with data copied from INIT
  // (which should contain the data in standard row-major C order).
  //
  Matrix (unsigned columns, unsigned rows, const std::vector<T> &init)
    : _columns (columns), _rows (rows), _data (init)
  { }

  // Copy-constructor.
  //
  Matrix (const Matrix &mat)
    : _columns (mat._columns), _rows (mat._rows), _data (mat._data)
  { }

  // COL,ROW element-access operator.
  //
  T &operator() (unsigned col, unsigned row)
  {
    return _data[row * _columns + col];
  }
  const T &operator() (unsigned col, unsigned row) const
  {
    return _data[row * _columns + col];
  }

  // Matrix sizes.
  //
  unsigned columns () const { return _columns; }
  unsigned rows () const { return _rows; }

  // Return a raw pointer to the matrix data, which is stored in
  // standard row-major C order.
  //
  T *data () { return _data.data (); }
  const T *data () const { return _data.data (); }

  // MATRIX * SCALAR multiplication operator.
  //
  Matrix operator* (T scale) const;

  // MATRIX * MATRIX multiplication operator.
  //
  Matrix operator* (const Matrix &mat) const;

  // Return the transpose (rows and columns interchanged) of this matrix.
  //
  Matrix transpose () const;

private:

  unsigned _columns, _rows;
  std::vector<T> _data;
};


// SCALAR * MATRIX multiplication operator.
//
template<typename T>
Matrix<T> operator* (T scale, const Matrix<T> &mat) { return mat * scale; }


}


// Include method definitions
//
#include "matrix.tcc"


#endif // __MATRIX_H__
