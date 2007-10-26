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

  GaussFilt (float _alpha = DEFAULT_ALPHA, float _width = DEFAULT_WIDTH)
    : Filter (_width), alpha (_alpha),
      _exp (exp (-alpha * width * width))
  { }
  GaussFilt (const ValTable &params)
    : Filter (params, DEFAULT_WIDTH),
      alpha (params.get_float ("alpha,a", DEFAULT_ALPHA)),
      _exp (exp (-alpha * width * width))
  { }

  virtual float val (float x, float y) const
  {
    return gauss1 (x) * gauss1 (y);
  }

  float gauss1 (float offs) const
  {
    return max (exp (-alpha * offs * offs) - _exp, 0.f);
  }

  float alpha;

private:

  float _exp;
};


}

#endif /* __GAUSS_FILT_H__ */


// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
