// matrix-funs.h -- Miscellanous matrix functions
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

#ifndef __MATRIX_FUNS_H__
#define __MATRIX_FUNS_H__

#include "matrix.h"


namespace snogray {


// Return the cholesky decomposition, L, of the matrix M.  L is a
// lower-triangular matrix such that L * L^T = M.
//
template<typename T>
Matrix<T> cholesky_decomposition (const Matrix<T> &M);

// Given an lower-triangular matrix L, and a column-matrix b, solve
// the equation L * x = b for x, and return the column-matrix x.
//
template<typename T>
Matrix<T> forward_substitution (const Matrix<T> &L, const Matrix<T> &b);

// Given an upper-triangular matrix U, and a column-matrix b, solve
// the equation U * x = b for b, and return the column-matrix b.
//
template<typename T>
Matrix<T> back_substitution (const Matrix<T> &U, const Matrix<T> &b);


}


// Include definitions
//
#include "matrix-funs.tcc"


#endif // __MATRIX_FUNS_H__
