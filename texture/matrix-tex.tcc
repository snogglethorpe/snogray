// matrix-tex.tcc -- Methods for MatrixTex class
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

#ifndef __MATRIX_TEX_TCC__
#define __MATRIX_TEX_TCC__

#include "config.h"

#if HAVE_EXTERN_TEMPLATE
# include "color.h"
#endif

#include "matrix-tex.h"


namespace snogray {


template<typename T, typename DT>
MatrixTex<T,DT>::MatrixTex (const std::string &filename, const ValTable &params)
  : matrix (new TupleMatrix<T,DT> (filename, params)),
    interp (matrix->width, matrix->height)
{ }

template<typename T, typename DT>
MatrixTex<T,DT>::MatrixTex (const Ref<TupleMatrix<T, DT> > &contents)
  : matrix (contents), interp (matrix->width, matrix->height)
{ }

template<typename T, typename DT>
MatrixTex<T,DT>::MatrixTex (const TupleMatrix<T,DT> &base,
			    unsigned offs_x, unsigned offs_y,
			    unsigned w, unsigned h)
  : matrix (new TupleMatrix<T,DT> (base, offs_x, offs_y, w, h)),
    interp (matrix->width, matrix->height)
{ }

// Evaluate this texture at TEX_COORDS.
//
template<typename T, typename DT>
T
MatrixTex<T,DT>::eval (const TexCoords &tex_coords) const
{
  unsigned xi_lo, yi_lo, xi_hi, yi_hi;
  float x_lo_fr, y_lo_fr, x_hi_fr, y_hi_fr;
  interp.calc_params (tex_coords.uv, xi_lo, yi_lo, xi_hi, yi_hi,
		      x_lo_fr, y_lo_fr, x_hi_fr, y_hi_fr);

  // Interpolate between the 4 pixels surrounding (x, y).
  // No attempt is made to optimize the case where an pixel is hit
  // directly, as that's probably fairly rare.
  //
  return
    x_lo_fr * y_lo_fr * (*matrix) (xi_lo, yi_lo)
    + x_lo_fr * y_hi_fr * (*matrix) (xi_lo, yi_hi)
    + x_hi_fr * y_lo_fr * (*matrix) (xi_hi, yi_lo)
    + x_hi_fr * y_hi_fr * (*matrix) (xi_hi, yi_hi);
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "matrix-tex.cc".
// 
#if HAVE_EXTERN_TEMPLATE
EXTERN_TEMPLATE_EXTENSION extern template class MatrixTex<Color>;
EXTERN_TEMPLATE_EXTENSION extern template class MatrixTex<float>;
#endif


} // namespace snogray

#endif // __MATRIX_TEX_TCC__


// arch-tag: 6fc727fd-116b-4298-a740-7d0d49aab6c6
