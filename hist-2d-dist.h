// hist-2d-dist.h -- Sampling distribution based on a 2d histogram
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

#ifndef __HIST_2D_DIST_H__
#define __HIST_2D_DIST_H__

#include <vector>
#include <algorithm>

#include "hist-2d.h"


namespace snogray {


// A sampling distribution based on a 2d histogram.  This is useful for
// doing cheap re-sampling based on an arbitrary 2d input set.
//
class Hist2dDist
{
public:

  // This constructor allocates the necessary memory, but won't be
  // usable until a histogram has been specified using Hist2dDist::calc.
  //
  Hist2dDist (unsigned w, unsigned h)
    : width (w), height (h), size (w*h),
      column_width (1.f / width), row_height (1.f / height),
      whole_row_cumulative_sums (height),
      individual_row_cumulative_sums (size)
  { }

  // This constructor automatically copies the size from HIST, and
  // calculates the PDF.  No references to HIST is kept.
  //
  Hist2dDist (const Hist2d &hist)
    : width (hist.width), height (hist.height), size (height * width),
      column_width (1.f / width), row_height (1.f / height),
      whole_row_cumulative_sums (height),
      individual_row_cumulative_sums (size)
  { calc (hist); }

  // Calculate the PDF based from the histogram HIST.  HIST's size must
  // be the same as the size this object was created with.  No reference
  // to HIST is kept.
  //
  void calc (const Hist2d &hist)
  {
    // Note, the use of double-precision floats here is intentional --
    // HDR images can cause precision problems if single-precision
    // floats are used.

    unsigned row_offs;

    // Find sum of entire input array.
    //
    double bin_sum = 0;
    row_offs = 0;
    for (unsigned row = 0; row < height; row++)
      {
	for (unsigned col = 0; col < width; col++)
	  bin_sum += hist.bins[row_offs + col];
	row_offs += width;
      }

    // Find cumulative sums of entire rows, normalized to the range 0-1
    // (so the last row will always have a value of 1, except in the
    // degenerate case where all bins are zero).
    //
    double inv_bin_sum = (bin_sum == 0) ? 0 : 1 / bin_sum;
    double normalized_sum = 0;
    row_offs = 0;
    for (unsigned row = 0; row < height; row++)
      {
	for (unsigned col = 0; col < width; col++)
	  normalized_sum += hist.bins[row_offs + col] * inv_bin_sum;
	whole_row_cumulative_sums[row] = normalized_sum;
	row_offs += width;
      }

    // Find cumulative sums within each row, normalized to the range 0-1
    // (so for each row, the last column within the row will always have
    // value 1, except in the degenerate case where all bins are zero).
    //
    row_offs = 0;
    for (unsigned row = 0; row < height; row++)
      {
	double row_sum = 0;
	for (unsigned col = 0; col < width; col++)
	  row_sum += hist.bins[row_offs + col];

	double inv_row_sum = (row_sum == 0) ? 0 : 1 / row_sum;
	double normalized_row_sum = 0;
	for (unsigned col = 0; col < width; col++)
	  {
	    normalized_row_sum += hist.bins[row_offs + col] * inv_row_sum;
	    individual_row_cumulative_sums[row_offs + col] = normalized_row_sum;
	  }
	row_offs += width;
      }
  }

  // Return a sample of this distribution based on the random
  // variables in PARAM.  The PDF at the sample location is returned
  // in _PDF.
  //
  // The returned UV coordinates should have roughly the same
  // distribution as the input data (limited by the granularity of
  // the histogram).
  //
  UV sample (const UV &param, float &_pdf) const
  {
    unsigned col, row, row_offs;
    sample (param, col, row, row_offs);

    _pdf = pdf (col, row, row_offs);

    return UV (col * column_width + fmod (param.u, column_width),
	       row * row_height   + fmod (param.v, row_height));
  }

  // Return a sample of this distribution based on the random
  // variables in PARAM.
  //
  // The returned UV coordinates should have roughly the same
  // distribution as the input data (limited by the granularity of the
  // histogram).
  //
  UV sample (const UV &param) const
  {
    unsigned col, row, row_offs;
    sample (param, col, row, row_offs);

    return UV (col * column_width + fmod (param.u, column_width),
	       row * row_height   + fmod (param.v, row_height));
  }

  // Return the PDF of this distribution at location POS.
  //
  float pdf (const UV &pos) const
  {
    unsigned col = clamp (int (pos.u * width), 0, int (width) - 1);
    unsigned row = clamp (int (pos.v * height), 0, int (height) - 1);

    return pdf (col, row, row * width);
  }

  const unsigned width, height, size;
  const float column_width, row_height;

private:

  template<typename Ptr>
  unsigned find_pos_in_sorted_vec (float val, const Ptr &beg, unsigned len)
    const
  {
#if 0
    unsigned pos = 0;
    while (val > beg[pos])
      pos++;
    return pos;
#else
    // XXX std::lower_bound is much faster than a linear search for
    // large histograms, but slower for small ones; it'd be nice to
    // parameterize this somehow...
    //
    return std::lower_bound (beg, beg + len, val) - beg;
#endif
  }

  // Sample the histogram and return the coordinates of the chosen bin in
  // COL and ROW.  The offset of the beginning of the row in
  // INDIVIDUAL_ROW_CUMULATIVE_SUMS is also returned in ROW_OFFSET.
  //
  void sample (const UV &param, unsigned &col, unsigned &row,
	       unsigned &row_offs)
    const
  {
    float u = min (param.u, 1.f), v = min (param.v, 1.f);

    // look in y dir.
    //
    row = find_pos_in_sorted_vec (v, whole_row_cumulative_sums.begin(), height);

    // XXX this multiply actually uses a lot of time; it's nicer to
    // accumulate the row-offset while finding the right how, or maybe
    // keep a vector of row offsets?
    //
    row_offs = row * width;

    // look in x dir
    //
    col = find_pos_in_sorted_vec (
	    u, individual_row_cumulative_sums.begin() + row_offs, width);
  }

  // Return the PDF of this distribution for locations in the bin
  // located at (COL, ROW), where ROW_OFFS is the offset in
  // INDIVIDUAL_ROW_CUMULATIVE_SUMS of the beginning of the row.
  //
  float pdf (unsigned col, unsigned row, unsigned row_offs) const
  {
    // Probability of choosing this row.
    //
    // As WHOLE_ROW_CUMULATIVE_SUMS contains cumulative sums of
    // whole-row probabilities, the probability of this row is the
    // difference of this row's cumulative-sum entry minus the
    // previous row's entry.
    //
    float row_prob = whole_row_cumulative_sums[row];
    if (row != 0)
      row_prob -= whole_row_cumulative_sums[row - 1];

    // Probability of choosing this column in the row.  Similarly to
    // ROW_PROB, this as the difference of the entries for the current
    // and previous columns in INDIVIDUAL_ROW_CUMULATIVE_SUMS.
    //
    float col_prob = individual_row_cumulative_sums[row_offs + col];
    if (col != 0)
      col_prob -= individual_row_cumulative_sums[row_offs + col - 1];

    // Probability of choosing this bin, which is just the probability
    // of choosing this row (ROW_PROB) multiplied by the probability
    // of choosing this columin within the row (COL_PROB).
    //
    float bin_prob = row_prob * col_prob;

    // PDF = probability of choosing a bin / bin area.  Since we
    // consider the "total area" to be 1, then the bin area is just
    // 1 / the number of bins (which is SIZE).
    //
    return bin_prob * size;
  }

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

#endif // __HIST_2D_DIST_H__
