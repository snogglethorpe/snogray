// tex-linterp.h -- texture linear interpolation
//
//  Copyright (C) 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEX_LINTERP_H__
#define __TEX_LINTERP_H__

#include "uv.h"


namespace snogray {


// This is a common class for calculating parameters to do linear
// interpolation of matrix elements.
//
class MatrixLinterp
{
public:

  // Subclasses typicall do not know the proper scale until they have
  // initialized their other fields, so they should set u_scale and v_scale
  // fields later.
  //
  MatrixLinterp (unsigned _width, unsigned _height)
    : width (_width), height (_height),
      u_scale (width - 1), v_scale (height - 1)
  { }

  // Calculate interpolation coordinates and weights.  Subclasses must do
  // the actual interpolation.
  //
  void calc_params (const UV &uv,
		    unsigned &xi_lo, unsigned &yi_lo,
		    unsigned &xi_hi, unsigned &yi_hi,
		    float &x_lo_fr, float &y_lo_fr,
		    float &x_hi_fr, float &y_hi_fr)
    const
  {
    // Remap to range [0, 1)
    //
    float u = uv.u - floor (uv.u);
    float v = uv.v - floor (uv.v);

    float   x    = u * u_scale,  y    = v * v_scale;
    float   x_lo = floor (x),    y_lo = floor (y);

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
    if (yi_hi >= height)
      yi_hi -= height;

    yi_lo = height - yi_lo - 1;
    yi_hi = height - yi_hi - 1;
  }

  UV map (unsigned x, unsigned y) const 
  {
    return UV (float (x) / u_scale, float (y) / v_scale);
  }
  
private:

  unsigned width, height;

  float u_scale, v_scale;
};


}

#endif // __TEX_LINTERP_H__
