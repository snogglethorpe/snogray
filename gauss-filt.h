// gauss.h -- Gaussian filter
//
//  Copyright (C) 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_GAUSS_FILT_H
#define SNOGRAY_GAUSS_FILT_H

#include "filter.h"


namespace snogray {


// 2D gauss
//
class GaussFilt : public Filter
{
public:

  static float default_alpha () { return 2; }
  static float default_width () { return 2; }

  GaussFilt (float _alpha = default_alpha(), float _width = default_width())
    : Filter (_width), alpha (_alpha),
      _exp (exp (-alpha * width * width))
  { }
  GaussFilt (const ValTable &params)
    : Filter (params, default_width()),
      alpha (params.get_float ("alpha,a", default_alpha())),
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

#endif /* SNOGRAY_GAUSS_FILT_H */


// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
