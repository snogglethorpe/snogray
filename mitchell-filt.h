// mitchell-filt.h -- Mitchell filter
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MITCHELL_FILT_H__
#define __MITCHELL_FILT_H__

#include "filter.h"


namespace snogray {


// 2D mitchell filter
//
class MitchellFilt : public Filter
{
public:

  static const float DEFAULT_B = 1.f / 3.f;
  static const float DEFAULT_C = 1.f / 3.f;
  static const float DEFAULT_WIDTH = 2;

  MitchellFilt (float _b = DEFAULT_B, float _c = DEFAULT_C,
		float _width = DEFAULT_WIDTH)
    : Filter (_width), b (_b), c (_c)
  { }
  MitchellFilt (const ValTable &params)
    : Filter (params, DEFAULT_WIDTH),
      b (params.get_float ("b", DEFAULT_B)),
      c (params.get_float ("c", DEFAULT_C))
  { }

  virtual float val (float x, float y) const
  {
    return mitchell1 (x) * mitchell1 (y);
  }

  float mitchell1 (float x) const
  {
    x = abs (2.f * x * inv_width);

    // This formula is from the PBRT book
    //
    if (x > 1.f)
      return ((-b - 6*c) * x*x*x + (6*b + 30*c) * x*x +
	      (-12*b - 48*c) * x + (8*b + 24*c)) * (1.f/6.f);
    else
      return ((12 - 9*b - 6*c) * x*x*x +
	      (-18 + 12*b + 6*c) * x*x +
	      (6 - 2*b)) * (1.f/6.f);
  }

  float b, c;
};


}

#endif /* __MITCHELL_FILT_H__ */


// arch-tag: 7fd2ef94-7377-449a-a24a-dfbea8c016bc
