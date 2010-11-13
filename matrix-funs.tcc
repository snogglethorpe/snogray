// matrix-funs.tcc -- Miscellanous matrix functions
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

#ifndef __MATRIX_FUNS_TCC__
#define __MATRIX_FUNS_TCC__

#include "snogassert.h"
#include "snogmath.h"


namespace snogray {



// cholesky_decomposition 

// Return the cholesky decomposition, L, of the matrix M.  L is a
// lower-triangular matrix such that L * L^T = M.
//
template<typename T>
Matrix<T>
cholesky_decomposition (const Matrix<T> &M)
{
  ASSERT (M.rows() == M.columns());

  unsigned size = M.rows();
  Matrix<T> L (size, size);

  // Formula (first index is column, second is row):
  //
  //    L00   0   0
  //    L01 L11   0 ...
  //    L02 L12 L22
  //       ...
  //
  // for j < i:  Lji = (1/Ljj) * (Mji - sum(k=0; k<j) {Lki * Lkj})
  //             Lii = sqrt (Mii - sum(k=0; k<i) {Lki^2})
  // for j > i:  Lji = 0
  //
  // =>
  //   L00 = sqrt (M00)
  //   L01 = (1/L00) * (M01)
  //   L11 = sqrt (M11 - L01^2)
  //   L02 = (1/L00) * (M02)
  //   L12 = (1/L11) * (M12 - L02*L01)
  //   L22 = sqrt (M22 - (L02^2 + L12^2))
  //   ...

  for (unsigned i = 0; i < size; i++)
    {
      // Compute Lji for j < i
      //
      for (unsigned j = 0; j < i; j++)
	{
	  float sum_Lki_Lkj = 0;
	  for (unsigned k = 0; k < i; k++)
	    sum_Lki_Lkj += L(k,i) * L(k,j);

	  L(j,i) = (M(j,i) - sum_Lki_Lkj) / L(j,j);
	}

      // Compute Lii
      //
      float sum_Lki_sq = 0;
      for (unsigned k = 0; k < i; k++)
	sum_Lki_sq += L(k,i)*L(k,i);

      L(i,i) = sqrt (M(i,i) - sum_Lki_sq);

      // Fill in Lji with zero for j > i
      //
      for (unsigned j = i + 1; j < size; j++)
	L(j,i) = 0;
    }

  return L;
}


// forward_substitution

// Given an lower-triangular matrix L, and a matrix B with the same
// number of rows, solve the equation L * X = B for X, and return the
// matrix X (which will have the same dimensions as B).
//
template<typename T>
Matrix<T>
forward_substitution (const Matrix<T> &L, const Matrix<T> &B)
{
  ASSERT (L.rows() == L.columns());
  ASSERT (B.rows() == L.rows());

  unsigned size = L.rows();
  unsigned num_eqns = B.columns();

  Matrix<T> X (num_eqns, size);

  // Each column of B represents a separate equation, which we solve
  // one by one.
  //
  for (unsigned eqn = 0; eqn < num_eqns; eqn++)
    {
      // Since L is a lower-triangular matrix, the expanded form of
      // the matrix equation is:
      //
      //    L0,0*x0 +       0 +       0 + 0 + ... + 0 = b0
      //    L0,1*x0 + L1,1*x1 +       0 + 0 + ... + 0 = b1
      //    L0,2*x0 + L1,2*x1 + L2,2*x2 + 0 + ... + 0 = b2
      //                          ...
      //    L0,n*x0 + L1,n*x1 +     ...     + Ln,n*xn = bn
      //
      // This allows x0 to be solved for using the first row only, and
      // then x1 to be solved using the second row plus the previously
      // computed value of x0.  Similarly, xn can be solved using row
      // n plus the results xi for i<n computed using previous rows.
      //

      // The computation starts with the first row, and moves forwards.
      //
      for (unsigned row = 0; row < size; row++)
	{
	  // This row represents the equation:
	  //
	  //   L(0,row)*x(0) + ... + L(row,row)*x(row) + 0 + ... + 0 = b(row)
	  //
	  // As we've already computed x(i) for all i < row, we can just
	  // compute the sum of L(i,row)*x(i) for all i < row, and then
	  // compute x(row) as:
	  //
	  //   x(row) = (b(row) - SUM) / L(row,row)
	  //
	  float sum = 0;
	  for (unsigned col = 0; col < row; col++)
	    sum += L (col, row) * X (eqn, col);

	  // Now compute X(row).
	  //
	  X (eqn, row) = (B (eqn, row) - sum) / L (row, row);
	}
    }

  return X;
}


// back_substitution

// Given an upper-triangular matrix U, and a matrix B with the same
// number of rows, solve the equation U * X = B for X, and return the
// matrix X (which will have the same dimensions as B).
//
template<typename T>
Matrix<T>
back_substitution (const Matrix<T> &U, const Matrix<T> &B)
{
  ASSERT (U.rows() == U.columns());
  ASSERT (B.rows() == U.rows());

  unsigned size = U.rows();
  unsigned num_eqns = B.columns();

  Matrix<T> X (num_eqns, size);

  // Each column of B represents a separate equation, which we solve
  // one by one.
  //
  for (unsigned eqn = 0; eqn < num_eqns; eqn++)
    {
      // Since U is an upper-triangular matrix, the expanded form of the
      // matrix equation is:
      //
      //    L0,0*x0 + L1,0*x1 + ...                 ... + Ln,0 * xn = bn
      //                                  ...
      //    0 + ... + 0 + Ln-2,n-2*xn-2 + Ln-1,n-2*xn-1 + Ln,n-2*xn = bn
      //    0 + ... + 0 +             0 + Ln-1,n-1*xn-1 + Ln,n-1*xn = bn
      //    0 + ... + 0 +                             0 + Ln,n  *xn = bn
      //
      // where n = size - 1.
      //
      // This allows xn to be solved for using the last row only, and then
      // xn-1 to be solved using the last row plus the previously computed
      // value of xn.  Similarly, xn can be solved using row n plus the
      // results xi for i>n computed using following rows.
      //

      // (note we need to use int indices because the terminating value is -1)

      // The computation starts with the last row, and moves backwards.
      //
      for (int row = size - 1; row >= 0; row--)
	{
	  // This row represents the equation:
	  //
	  //    0 + ... + 0 + U(row,row)*X(row) + ... + U(n,row)*X(n) = b(row)
	  //
	  // where n = size - 1.
	  //
	  // As we've already computed x(i) for all i > row, we can just
	  // compute the sum of U(i,row)*x(i) for all i > row, and then
	  // compute x(row) as:
	  //
	  //   x(row) = (b(row) - SUM) / U(row,row)
	  //
	  float sum = 0;
	  for (int col = size - 1; col > row; col--)
	    sum += U (col, row) * X (eqn, col);

	  // Now compute X(row).
	  //
	  X (eqn, row) = (B (eqn, row) - sum) / U (row, row);
	}
    }

  return X;
}


}

#endif // __MATRIX_FUNS_TCC__
