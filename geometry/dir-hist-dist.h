// dir-hist-dist.h -- Sampling distribution based on a directional histogram
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_DIR_HIST_DIST_H
#define SNOGRAY_DIR_HIST_DIST_H

#include "util/snogmath.h"

#include "dir-hist.h"
#include "hist-2d-dist.h"


namespace snogray {


// A sampling distribution based on a directional histogram.  This is
// useful for doing cheap re-sampling based on an arbitrary set of
// directions.
//
class DirHistDist : public Hist2dDist
{
public:

  // This object won't be useful until a histogram has been set using
  // DirHistDist::set_histogram.
  //
  DirHistDist () { }

  // This constructor automatically copies the size from HIST, and
  // calculates the PDF.  No references to HIST is kept.
  //
  DirHistDist (const DirHist &hist) : Hist2dDist (hist) { }

  // Return a sample of this distribution based on the random
  // variables in PARAM.  The PDF at the sample location is returned
  // in _PDF.
  //
  // The returned direction should have roughly the same
  // distribution as the input data (limited by the granularity of
  // the histogram).
  //
  Vec sample (const UV &param, float &_pdf) const
  {
    UV pos = Hist2dDist::sample (param, _pdf);

    // Adjust _PDF to reflect that we're sampling over the surface
    // of a sphere.
    //
    _pdf *= 0.25f * INV_PIf;

    // Convert the sampled position to a direction.
    //
    return DirHist::pos_to_dir (pos);
  }

  // Return a sample of this distribution based on the random
  // variables in PARAM.
  //
  // The returned direction should have roughly the same
  // distribution as the input data (limited by the granularity of
  // the histogram).
  //
  Vec sample (const UV &param) const
  {
    UV pos = Hist2dDist::sample (param);

    // Convert the sampled position to a direction.
    //
    return DirHist::pos_to_dir (pos);
  }

  // Return the PDF of this distribution in direction DIR.
  //
  float pdf (const Vec &dir) const
  {
    return Hist2dDist::pdf (DirHist::dir_to_pos (dir)) * 0.25f * INV_PIf;
  }
};


}

#endif // SNOGRAY_DIR_HIST_DIST_H
