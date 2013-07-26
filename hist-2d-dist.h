// hist-2d-dist.h -- Sampling distribution based on a 2d histogram
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

#ifndef SNOGRAY_HIST_2D_DIST_H
#define SNOGRAY_HIST_2D_DIST_H

#include <vector>
#include <algorithm>

#include "compiler.h"

#include "hist-2d.h"


namespace snogray {


// A sampling distribution based on a 2d histogram.  This is useful for
// doing cheap re-sampling based on an arbitrary 2d input set.
//
class Hist2dDist
{
public:

  // Construct a default object, which is equivalent to all zeroes.
  // A histogram can later be added using Hist2dDist::set_histogram.
  //
  Hist2dDist ()
    : width (0), height (0), size (0),
      column_width (0), row_height (0),
      whole_row_cumulative_sums (0),
      individual_row_cumulative_sums (0)
  { }

  // This constructor copies the size from HIST, and calculates the
  // PDF.  No reference to HIST is kept.
  //
  Hist2dDist (const Hist2d &hist);

  // Calculate the PDF based from the histogram HIST.  No reference to
  // HIST is kept.
  //
  void set_histogram (const Hist2d &hist);

  // Return a sample of this distribution based on the random
  // variables in PARAM.  The PDF at the sample location is returned
  // in _PDF.
  //
  // The returned UV coordinates should have roughly the same
  // distribution as the input data (limited by the granularity of
  // the histogram).
  //
  UV sample (const UV &param, float &_pdf) const;

  // Return a sample of this distribution based on the random
  // variables in PARAM.
  //
  // The returned UV coordinates should have roughly the same
  // distribution as the input data (limited by the granularity of the
  // histogram).
  //
  UV sample (const UV &param) const;

  // Return the PDF of this distribution at location POS.
  //
  float pdf (const UV &pos) const;

  unsigned width, height, size;
  float column_width, row_height;

private:

  // Calculate the PDF based from the histogram HIST.  HIST's size
  // must be the same as this object's current size.  No reference to
  // HIST is kept.
  //
  void calc (const Hist2d &hist);

  // Sample the histogram and return the coordinates of the chosen
  // bin in COL and ROW.  The offset of the beginning of the row in
  // INDIVIDUAL_ROW_CUMULATIVE_SUMS is also returned in ROW_OFFSET.
  //
  // Normally the function return value is true, but in the rare
  // case where sampling is impossible because _all_ the data was
  // zero, false is returned instead (and all other return values
  // are undefined).
  //
  bool sample (const UV &param, unsigned &col, unsigned &row,
	       unsigned &row_offs)
    const;

  // Return the PDF of this distribution for locations in the bin
  // located at (COL, ROW), where ROW_OFFS is the offset in
  // INDIVIDUAL_ROW_CUMULATIVE_SUMS of the beginning of the row.
  //
  float pdf (unsigned col, unsigned row, unsigned row_offs) const;

  // Cumulative sum of whole-row probabilities.  Each entry is the
  // probability of choosing that row or any row before it (so the
  // last entry is always 1).
  //
  std::vector<float> whole_row_cumulative_sums;

  // For each row, the cumulative sum of column probabilities for
  // that row.  Each entry is the probability of choosing that colum
  // in the row (assuming the row is chosen) or any column before it
  // (so the last entry for each row is always 1).
  //
  std::vector<float> individual_row_cumulative_sums;
};


}

#endif // SNOGRAY_HIST_2D_DIST_H
