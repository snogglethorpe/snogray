// matrix-funs.tcc -- Miscellanous matrix functions
//
//  Copyright (C) 2010, 2012  Miles Bader <miles@gnu.org>
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

#include <algorithm>

#include "snogassert.h"
#include "snogmath.h"


namespace snogray {



// ----------------------------------------------------------------
// cholesky_decomposition 


// Return the cholesky decomposition, L, of the positive-definite
// symmetric matrix M.  L is a lower-triangular matrix such that
// L * L^T = M.
//
// If M is not positive-definite and symmetric, an empty matrix
// will be returned instead.
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
      // Compute Lij for j < i
      //
      for (unsigned j = 0; j < i; j++)
	{
	  T sum_Lik_Ljk = 0;
	  for (unsigned k = 0; k < i; k++)
	    sum_Lik_Ljk += L(i,k) * L(j,k);

	  L(i,j) = (M(i,j) - sum_Lik_Ljk) / L(j,j);
	}

      // Compute Lii
      //
      T sum_Lik_sq = 0;
      for (unsigned k = 0; k < i; k++)
	sum_Lik_sq += L(i,k)*L(i,k);

      L(i,i) = sqrt (M(i,i) - sum_Lik_sq);

      // If decomposition failed, return a failure indicator.
      //
      if (L(i,i) == 0)
	{
	  L.clear ();
	  break;
	}

      // Fill in Lij with zero for j > i
      //
      for (unsigned j = i + 1; j < size; j++)
	L(i,j) = 0;
    }

  return L;
}



// ----------------------------------------------------------------
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

  Matrix<T> X (size, num_eqns);

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
	  T sum = 0;
	  for (unsigned col = 0; col < row; col++)
	    sum += L (row, col) * X (col, eqn);

	  // Now compute X(row).
	  //
	  X (row, eqn) = (B (row, eqn) - sum) / L (row, row);
	}
    }

  return X;
}



// ----------------------------------------------------------------
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

  Matrix<T> X (size, num_eqns);

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
	  T sum = 0;
	  for (int col = size - 1; col > row; col--)
	    sum += U (row, col) * X (col, eqn);

	  // Now compute X(row).
	  //
	  X (row, eqn) = (B (row, eqn) - sum) / U (row, row);
	}
    }

  return X;
}



// ----------------------------------------------------------------
// pseudo-inverse


// Return the Moore-Penrose pseudo-inverse, M+, of M.
//
// If there is no pseudo-inverse, an empty matrix is returned.
//
template<typename T>
Matrix<T> pseudo_inverse (const Matrix<T> &M)
{
  // Transpose of M.
  //
  const Matrix<T> Mt = M.transpose ();

  // If M has more columns than rows, then (M^T * M) is guaranteed to
  // not have full row-rank (it will be a square matrix with size
  // max(M.columns, M.rows), which I guess lowers the information
  // density), and that will cause cholesky_decomposition to fail.
  //
  // To avoid that, we actually operate on N and Nt, where N is
  // whichever of M or Mt has fewer columns than rows (think of it
  // meaning "Narrow"), and Nt its transpose.  Thus (N^T * N) will be
  // a square matrix with size min(N.columns, N.rows), and so stands a
  // better chance of having full rank.  In the case where N is Mt,
  // then the resulting pseudo-inverse will be the transpose of M's
  // pseudo-inverse, so we transpose the return value.
  //
  // cholesky_decomposition may still fail, but at least it's not
  // guaranteed.
  //
  bool swapped = (M.columns() > M.rows());
  const Matrix<T> &N = swapped ? Mt : M;
  const Matrix<T> &Nt = swapped ? M : Mt;

  // Compute cholesky decomposition, R, of (N^T * N).
  // Then R will be a upper-triangular matrix such that R^T * R = N^T * N.
  //
  // The cholesky_decomposition function actually returns a
  // lower-triangular matrix which is just the transpose of R.
  //
  Matrix<T> Rt = cholesky_decomposition (Nt * N);

  // cholesky_decomposition may have failed -- which it signals by
  // returning an empty matrix -- if (N^T * N) had linearly-dependent
  // columns.  In that case, we try again using (N * N^T); if that
  // succeeds using, we can just proceed as normal and transpose the
  // final result.  If that fails too, we just return an empty matrix.
  //
  if (Rt.empty ())
    return Matrix<T> ();

  Matrix<T> R = Rt.transpose ();

  // As R^T is a lower-triangular matrix, we can use a forward-
  // substitution step to solve the equation R^T * z = N^T, for z.
  //
  Matrix<T> z = forward_substitution (Rt, Nt);

  // Similarly, as R is an upper-triangular matrix, we can use a
  // back-substitution step to solve the equation R * N+ = z, for N+.
  //
  Matrix<T> Np = back_substitution (R, z);

  if (swapped)
    Np = Np.transpose ();

  return Np;
}


}

#endif // __MATRIX_FUNS_TCC__
