// matrix-tex2.tcc -- Methods for MatrixTex2 class
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_TEX2_TCC__
#define __MATRIX_TEX2_TCC__

#include "config.h"

#if HAVE_EXTERN_TEMPLATE
# include "color.h"
#endif

#include "matrix-tex2.h"


namespace Snogray {


template<typename T>
MatrixTex2<T>::MatrixTex2 (const std::string &filename)
  : matrix (filename), interp (matrix.width, matrix.height)
{ }

template<typename T>
MatrixTex2<T>::MatrixTex2 (const TupleMatrix<T> &base,
			   unsigned offs_x, unsigned offs_y,
			   unsigned w, unsigned h)
  : matrix (base, offs_x, offs_y, w, h), interp (matrix.width, matrix.height)
{ }

template<typename T>
T
MatrixTex2<T>::map (tparam_t u, tparam_t v) const
{
  unsigned xi_lo, yi_lo, xi_hi, yi_hi;
  float x_lo_fr, y_lo_fr, x_hi_fr, y_hi_fr;
  interp.calc_params (u, v,
		      xi_lo, yi_lo, xi_hi, yi_hi,
		      x_lo_fr, y_lo_fr, x_hi_fr, y_hi_fr);

  // Interpolate between the 4 pixels surrounding (x, y).
  // No attempt is made to optimize the case where an pixel is hit
  // directly, as that's probably fairly rare.
  //
  return
    x_lo_fr * y_lo_fr * matrix (xi_lo, yi_lo)
    + x_lo_fr * y_hi_fr * matrix (xi_lo, yi_hi)
    + x_hi_fr * y_lo_fr * matrix (xi_hi, yi_lo)
    + x_hi_fr * y_hi_fr * matrix (xi_hi, yi_hi);
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "matrix-tex2.cc".
// 
#if HAVE_EXTERN_TEMPLATE
extern template class MatrixTex2<Color>;
#endif


} // namespace Snogray

#endif // __MATRIX_TEX2_TCC__


// arch-tag: 6fc727fd-116b-4298-a740-7d0d49aab6c6
