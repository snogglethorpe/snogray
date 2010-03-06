// dir-hist.h -- Directional histogram
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

#ifndef __DIR_HIST_H__
#define __DIR_HIST_H__

#include "uv.h"
#include "hist-2d.h"
#include "spherical-coords.h"


namespace snogray {


// A 2d histogram for holding directional vectors.  This is just a 2d
// histogram that maps input vectors to the surface of a sphere.
//
class DirHist : public Hist2d
{
public:

  DirHist (unsigned w, unsigned h) : Hist2d (w, h) { }

  using Hist2d::add;

  // Record an input sample with direction DIR and value VAL.
  //
  void add (const Vec &dir, float val)
  {
    add (dir_to_pos (dir), val);
  }

  // Return the position in the underlying 2d histogram
  // corresponding to direction DIR.
  //
  static UV dir_to_pos (const Vec &dir)
  {
    return z_axis_spherical (dir);
  }

  // Return the direction corresponding to the position POS in the
  // underlying 2d histogram.
  //
  static Vec pos_to_dir (const UV &pos)
  {
    return z_axis_spherical_to_vec (pos);
  }
};


}

#endif // __DIR_HIST_H__
