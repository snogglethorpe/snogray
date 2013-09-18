// dir-hist.h -- Directional histogram
//
//  Copyright (C) 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_DIR_HIST_H
#define SNOGRAY_DIR_HIST_H

#include "uv.h"
#include "sphere-sample.h"

#include "hist-2d.h"


namespace snogray {


// A 2d histogram for holding directional vectors.  This is just a 2d
// histogram that maps input vectors to the surface of a sphere.
//
class DirHist : public Hist2d
{
public:

  DirHist (unsigned size) : Hist2d (size * 2, size) { }
  DirHist (unsigned w, unsigned h) : Hist2d (w, h) { }

  using Hist2d::add;

  // Record an input sample with direction DIR and value VAL.
  //
  void add (const Vec &dir, float val)
  {
    add (dir_to_pos (dir), val);
  }

  //
  // Methods to translate between direction vectors and histogram
  // coordinates.
  //
  // Given a radius 1 sphere around the origin corresponding to the
  // set of directions, we want every bin in our underlying 2d
  // histogram to map to the same amount of surface area on the
  // sphere.
  //
  // This is done using sphere_sample and sphere_sample_inverse, which
  // do equal-area mapping between UV coordinates and directions.
  //

  // Return the position in the underlying 2d histogram
  // corresponding to direction DIR.  DIR must be a unit vector.
  //
  static UV dir_to_pos (const Vec &dir) { return sphere_sample_inverse (dir); }

  // Return the direction corresponding to the position POS in the
  // underlying 2d histogram.
  //
  static Vec pos_to_dir (const UV &pos) { return sphere_sample (pos); }
};


}

#endif // SNOGRAY_DIR_HIST_H
