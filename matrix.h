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

  Matrix (unsigned columns, unsigned rows)
    : _columns (columns), _rows (rows),
      _data (columns * rows)	// default-initialized!
  { }

  Matrix (const Matrix &mat)
    : _columns (mat._columns), _rows (mat._rows), _data (mat._data)
  { }

  T &operator() (unsigned col, unsigned row)
  {
    return _data[row * _columns + col];
  }
  const T &operator() (unsigned col, unsigned row) const
  {
    return _data[row * _columns + col];
  }

  unsigned columns () const { return _columns; }
  unsigned rows () const { return _rows; }

  T *data () { return _data; }
  const T *data () const { return _data; }

  Matrix operator* (T scale) const;

  Matrix operator* (const Matrix &mat) const;

  Matrix transpose () const;

private:

  unsigned _columns, _rows;
  std::vector<T> _data;
};


template<typename T>
Matrix<T> operator* (T scale, const Matrix<T> &mat) { return mat * scale; }


}


// Include method definitions
//
#include "matrix.tcc"


#endif // __MATRIX_H__
