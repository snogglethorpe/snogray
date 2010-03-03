// hist-2d.h -- 2d histogram
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

#ifndef __HIST_2D_H__
#define __HIST_2D_H__

#include <cstring> // memset
#include <vector>

#include "snogmath.h"
#include "uv.h"


namespace snogray {


// A 2d histogram.
//
class Hist2d
{
public:

  Hist2d (unsigned w, unsigned h)
    : width (w), height (h), size (w*h), bins (size, 0)
  { }

  // Reset all state, to prepare for new input data.
  //
  void clear ()
  {
#if 0
    std::fill (bins.begin(), bins.end(), 0);
#else
    memset (&bins[0], 0, size * sizeof (bins[0]));
#endif
  }

  // Record an input sample with position POS and value VAL.
  //
  void add (const UV &pos, float val)
  {
    unsigned col = min (unsigned (pos.u * width), width-1);
    unsigned row = min (unsigned (pos.v * height), height-1);
    bins[col + row*width] += val;
  }

  // Record an input sample with position (COL, ROW) in integer bin
  // coordinates, and value VAL.
  //
  void add (unsigned col, unsigned row, float val)
  {
    bins[col + row*width] += val;
  }

  float &operator() (unsigned col, unsigned row)
  {
    return bins[col + row*width];
  }
  const float &operator() (unsigned col, unsigned row) const
  {
    return bins[col + row*width];
  }

  const unsigned width, height, size;

  std::vector<float> bins;
};


}

#endif // __HIST_2D_H__
