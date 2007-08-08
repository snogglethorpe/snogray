// gauss.h -- Gaussian filter
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GAUSS_FILT_H__
#define __GAUSS_FILT_H__

#include "filter.h"


namespace snogray {


// 2D gauss
//
class GaussFilt : public Filter
{
public:

  static const float DEFAULT_ALPHA = 2;
  static const float DEFAULT_WIDTH = 2;

  GaussFilt (float _alpha = DEFAULT_ALPHA,
	     float _x_width = DEFAULT_WIDTH, float _y_width = DEFAULT_WIDTH)
    : Filter (_x_width, _y_width), alpha (_alpha),
      x_exp (exp (-alpha * x_width * x_width)),
      y_exp (exp (-alpha * y_width * y_width))
  { }
  GaussFilt (const ValTable &params)
    : Filter (params, DEFAULT_WIDTH),
      alpha (params.get_float ("alpha,a", DEFAULT_ALPHA)),
      x_exp (exp (-alpha * x_width * x_width)),
      y_exp (exp (-alpha * y_width * y_width))
  { }

  virtual float val (float x, float y) const
  {
    return gauss1 (x, x_exp) * gauss1 (y, y_exp);
  }

  float gauss1 (float offs, float ex) const
  {
    return max (exp (-alpha * offs * offs) - ex, 0.f);
  }

  float alpha;

  float x_exp, y_exp;
};


}

#endif /* __GAUSS_FILT_H__ */


// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
