// triangle.h -- Triangleian filter
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRIANGLE_FILT_H__
#define __TRIANGLE_FILT_H__

#include "filter.h"


namespace snogray {


class TriangleFilt : public Filter
{
public:

  static const float DEFAULT_WIDTH = 2;

  TriangleFilt (float _width = DEFAULT_WIDTH) : Filter (_width) { }
  TriangleFilt (const ValTable &params) : Filter (params, DEFAULT_WIDTH) { }

  virtual float val (float x, float y) const
  {
    return max (0.f, width - abs (x)) * max (0.f, width - abs (y));
  }
};


}

#endif // __TRIANGLE_FILT_H__
