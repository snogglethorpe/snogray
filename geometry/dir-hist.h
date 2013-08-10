// dir-hist.h -- Directional histogram
//
//  Copyright (C) 2010-2012  Miles Bader <miles@gnu.org>
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


  //
  // Methods to translate between direction vectors and histogram
  // coordinates.
  //
  // Given a radius-1 sphere around the origin corresponding to the
  // set of directions, we want every bin in our underlying 2d
  // histogram to map to the same amount of surface area on the
  // sphere.
  //
  // To do this, we use the same approach used by "sample_cone" (in
  // cone-sample.h, which see):  U is mapped linearly to the angle
  // around the z-axis (i.e., it's the "longitude"), and V is mapped
  // to the z-coordinate of the point where the direction vector
  // hits the sphere.
  //
  // Thus as direction vectors approach the z-axis, they get closer
  // together around in the U direction, but farther apart in the V
  // direction, by exactly the same amount.
  //

  // Return the position in the underlying 2d histogram
  // corresponding to direction DIR.  DIR must be a unit vector.
  //
  static UV dir_to_pos (const Vec &dir)
  {
    return UV (clamp01 (float (atan2 (dir.y, dir.x)) * (INV_PIf / 2) + 0.5f),
	       clamp01 (float (1 - dir.z) / 2));
  }

  // Return the direction corresponding to the position POS in the
  // underlying 2d histogram.
  //
  static Vec pos_to_dir (const UV &pos)
  {
    float z = 1 - pos.v * 2;
    float r = sqrt (max (1 - z * z, 0.f));
    float phi = (pos.u - 0.5f) * 2 * PIf;
    float x = r * cos (phi);
    float y = r * sin (phi);
    return Vec (x, y, z);
  }
};


}

#endif // SNOGRAY_DIR_HIST_H
