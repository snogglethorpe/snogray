// tex-linterp.h -- texture linear interpolation
//
//  Copyright (C) 2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TEX_LINTERP_H
#define SNOGRAY_TEX_LINTERP_H

#include "geometry/uv.h"


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
      u_scale (width), v_scale (height)
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

    float   x    = u * u_scale - 0.5f,  y    = v * v_scale - 0.5f;
    float   x_lo = floor (x),           y_lo = floor (y);

    x_hi_fr = x - x_lo;
    y_hi_fr = y - y_lo;
    x_lo_fr = 1 - x_hi_fr;
    y_lo_fr = 1 - y_hi_fr;

    int xi_lo_unwr = int (x_lo);
    int yi_lo_unwr = int (y_lo);
    xi_hi = xi_lo_unwr + 1;
    yi_hi = yi_lo_unwr + 1;

    if (xi_lo_unwr < 0)
      xi_lo_unwr += width;
    if (yi_lo_unwr < 0)
      yi_lo_unwr += height;
    if (xi_hi >= width)
      xi_hi -= width;
    if (yi_hi >= height)
      yi_hi -= height;

    xi_lo = unsigned (xi_lo_unwr);
    yi_lo = unsigned (yi_lo_unwr);

    yi_lo = height - yi_lo - 1;
    yi_hi = height - yi_hi - 1;
  }

  UV map (unsigned x, unsigned y) const 
  {
    return UV ((float (x) + 0.5f) / u_scale,
	       (float (y) + 0.5f) / v_scale);
  }
  
private:

  unsigned width, height;

  float u_scale, v_scale;
};


}

#endif // SNOGRAY_TEX_LINTERP_H
