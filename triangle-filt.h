// triangle.h -- Triangleian filter
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TRIANGLE_FILT_H
#define SNOGRAY_TRIANGLE_FILT_H

#include "filter.h"


namespace snogray {


class TriangleFilt : public Filter
{
public:

  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  static float default_width () { return 2; }

  TriangleFilt (float _width = default_width()) : Filter (_width) { }
  TriangleFilt (const ValTable &params) : Filter (params, default_width()) { }

  virtual float val (float x, float y) const
  {
    return max (0.f, x_width - abs (x)) * max (0.f, y_width - abs (y));
  }
};


}

#endif // SNOGRAY_TRIANGLE_FILT_H
