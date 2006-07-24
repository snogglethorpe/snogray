// gauss.h -- Gaussian filter
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GAUSS_H__
#define __GAUSS_H__

#include "filter.h"

namespace Snogray {

// 2D gauss
//
class Gauss : public Filter
{
public:

  static const float DEFAULT_ALPHA = 2;
  static const float DEFAULT_WIDTH = 2;

  Gauss (float _alpha = DEFAULT_ALPHA,
	 float _x_width = DEFAULT_WIDTH, float _y_width = DEFAULT_WIDTH)
    : Filter (_x_width, _y_width), alpha (_alpha),
      x_exp (exp (-alpha * x_width * x_width)),
      y_exp (exp (-alpha * y_width * y_width))
  { }
  Gauss (const Params &params)
    : Filter (params, DEFAULT_WIDTH),
      alpha (params.get_float ("alpha", DEFAULT_ALPHA)),
      x_exp (exp (-alpha * x_width * x_width)),
      y_exp (exp (-alpha * y_width * y_width))
  { }
  ~Gauss ();

  virtual float val (float x, float y) const;

  float gauss1 (float offs, float ex) const
  {
    return max (exp (-alpha * offs * offs) - ex, 0.f);
  }

  float alpha;

  float x_exp, y_exp;
};

}

#endif /* __GAUSS_H__ */

// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
