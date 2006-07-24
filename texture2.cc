// texture2.h -- 2d texture datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"

#include "texture2.h"

using namespace Snogray;

Color
Texture2::map (param_t u, param_t v)
{
  // Remap to range [0, 1)
  //
  u -= floor (u);
  v -= floor (v);

  param_t   x    = u * u_scale,  y    = v * v_scale;
  param_t   x_lo = floor (x),    y_lo = floor (y);

  param_t   x_hi_fr = x - x_lo,    y_hi_fr = y - y_lo;
  param_t   x_lo_fr = 1 - x_hi_fr, y_lo_fr = 1 - y_hi_fr;

  unsigned xi_lo = unsigned (x_lo), yi_lo = unsigned (y_lo);
  unsigned xi_hi = xi_lo + 1, yi_hi = yi_lo + 1;

  if (xi_hi >= image.width)
    xi_hi -= image.width;
  if (yi_hi >= image.width)
    yi_hi -= image.width;

  yi_lo = image.height - yi_lo - 1;
  yi_hi = image.height - yi_hi - 1;

  // Interpolate between the 4 pixels surrounding (x, y).
  // No attempt is made to optimize the case where an pixel is hit
  // directly, as that's probably fairly rare.
  //
  return
      x_lo_fr * y_lo_fr * image (xi_lo, yi_lo)
    + x_lo_fr * y_hi_fr * image (xi_lo, yi_hi)
    + x_hi_fr * y_lo_fr * image (xi_hi, yi_lo)
    + x_hi_fr * y_hi_fr * image (xi_hi, yi_hi);
}

// arch-tag: 1909d9b8-d35c-4256-bf90-2fc3e728e5e6
