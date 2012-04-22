// least-squares-fit.h -- Least-squares curve-fitting
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LEAST_SQUARES_FIT_H
#define SNOGRAY_LEAST_SQUARES_FIT_H

#include "matrix.h"
#include "matrix-funs.h"
#include "snogassert.h"


namespace snogray {


// Return a least-squares fit, B, for the formula X * B ~= Y.
//
// A least-squares fit minimizes the sum of the squares of the
// differences between elements of X * B and Y.
//
// Y should be a matrix with the same number of rows as X, where each
// column contains values y0...yn on a curve being fitted (if y has
// multiple columns, each will be handled independently).  X
// represents the set of functions being used to fit the curve, and
// can have as many columns as there are functions.  The answer B will
// be a matrix with as many rows as X has columns, and as many columns
// as Y.  Each column of B represents a set of scale factors, b0...bm
// such that:
//
//   b0*[X0,0, ... X0,n] + ... + bm-1 * [Xm-1,0, ... Xm-1,n] ~= [y0, ... yn]
//
// where b and y are corresponding single columns of B and Y.
//
// This function only works the matrix X^T * X has full rank, that is,
// all columns/rows of X^T * X are linearly independent.
//
template<typename T>
Matrix<T>
least_squares_fit (const Matrix<T> &X, const Matrix<T> &Y)
{
  ASSERT (X.rows() == Y.rows());

  // Transpose of X, X^T
  //
  Matrix<T> Xt = X.transpose ();

  // Compute cholesky decomposition, R, of (X^T * X).
  // Then R will be a upper-triangular matrix such that R^T * R = X^T * X.
  //
  // The cholesky_decomposition function actually returns a
  // lower-triangular matrix which is just the transpose of R.
  //
  Matrix<T> Rt = cholesky_decomposition (Xt * X);
  Matrix<T> R = Rt.transpose ();

  // As R^T is a lower-triangular matrix, we can use a forward-
  // substitution step to solve the equation R^T * Z = X^T * Y, for z.
  //
  Matrix<T> Z = forward_substitution (Rt, Xt * Y);

  // Similarly, as R is an upper-triangular matrix, we can use a
  // back-substitution step to solve the equation R * B = Z, for B.
  //
  Matrix<T> B = back_substitution (R, Z);

  // And B is the answer!
  //
  return B;
}


}


#endif // SNOGRAY_LEAST_SQUARES_FIT_H
