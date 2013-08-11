// matrix.h -- General-purpose matrix type
//
//  Copyright (C) 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MATRIX_H
#define SNOGRAY_MATRIX_H

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
  Matrix () : _rows (0), _columns (0) { }

  // Make a COLUMNS x ROWS sized matrix with uninitialized data.
  //
  Matrix (unsigned rows, unsigned columns)
    : _rows (rows), _columns (columns),
      _data (rows * columns)	// default-initialized!
  { }

  // Make a ROWS x COLUMNS sized matrix with data copied from INIT
  // (which should contain the data in standard row-major C order).
  //
  Matrix (unsigned rows, unsigned columns, const std::vector<T> &init)
    : _rows (rows), _columns (columns), _data (init)
  { }

  // Copy-constructor.
  //
  Matrix (const Matrix &mat)
    : _rows (mat._rows), _columns (mat._columns), _data (mat._data)
  { }

  // Make this into an empty matrix, by setting the number of rows and
  // columns to zero.
  //
  void clear ()
  {
    _rows = _columns = 0;
    _data.clear ();
  }

  // Return true if this is an empty matrix (zero rows and columns).
  // Such matrices are sometimes used to indicate errors.
  //
  bool empty ()
  {
    return _columns == 0 || _rows == 0;
  }

  // COL,ROW element-access operator.
  //
  T &operator() (unsigned row, unsigned col)
  {
    return _data[row * _columns + col];
  }
  const T &operator() (unsigned row, unsigned col) const
  {
    return _data[row * _columns + col];
  }

  // Matrix sizes.
  //
  unsigned rows () const { return _rows; }
  unsigned columns () const { return _columns; }

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

  unsigned _rows, _columns;
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


#endif // SNOGRAY_MATRIX_H
