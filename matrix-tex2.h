// matrix-tex2.h -- 2d texture based on discrete matrix of values
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_TEX2_H__
#define __MATRIX_TEX2_H__

#include <string>

#include "snogmath.h"
#include "tuple-matrix.h"
#include "tex2.h"


namespace Snogray {


// This is a common class for doing MatrixTex2 interpolation
//
class MatrixTex2Interp
{
public:

  // Subclasses typicall do not know the proper scale until they have
  // initialized their other fields, so they should set u_scale and v_scale
  // fields later.
  //
  MatrixTex2Interp (unsigned _width, unsigned _height)
    : width (_width), height (_height),
      u_scale (width - 1), v_scale (height - 1)
  { }

  // Calculate interpolation coordinates and weights.  Subclasses must do
  // the actual interpolation.
  //
  void calc_params (tparam_t u, tparam_t v,
		    unsigned &xi_lo, unsigned &yi_lo,
		    unsigned &xi_hi, unsigned &yi_hi,
		    float &x_lo_fr, float &y_lo_fr,
		    float &x_hi_fr, float &y_hi_fr)
    const
  {
    // Remap to range [0, 1)
    //
    u -= floor (u);
    v -= floor (v);

    tparam_t   x    = u * u_scale,  y    = v * v_scale;
    tparam_t   x_lo = floor (x),    y_lo = floor (y);

    x_hi_fr = x - x_lo;
    y_hi_fr = y - y_lo;
    x_lo_fr = 1 - x_hi_fr;
    y_lo_fr = 1 - y_hi_fr;

    xi_lo = unsigned (x_lo);
    yi_lo = unsigned (y_lo);
    xi_hi = xi_lo + 1;
    yi_hi = yi_lo + 1;

    if (xi_hi >= width)
      xi_hi -= width;
    if (yi_hi >= width)
      yi_hi -= width;

    yi_lo = height - yi_lo - 1;
    yi_hi = height - yi_hi - 1;
  }
  
private:

  unsigned width, height;

  tparam_t u_scale, v_scale;
};

// A 2d texture based on a matrix tuple (probably loaded from an image).
//
template<typename T>
class MatrixTex2 : public Tex2<T>
{
public:

  MatrixTex2 (const std::string &filename);
  MatrixTex2 (const TupleMatrix<T> &base,
	      unsigned offs_x = 0, unsigned offs_y = 0,
	      unsigned w = 0, unsigned h = 0);

  virtual T map (tparam_t u, tparam_t v) const;

private:

  // Matrix holding data for this texture.
  //
  TupleMatrix<T> matrix;

  const MatrixTex2Interp interp;
};


} // namespace Snogray

// Include method definitions
//
#include "matrix-tex2.tcc"

#endif // __MATRIX_TEX2_H__


// arch-tag: 0d92b346-d347-4025-aab0-cbaf594f6789
