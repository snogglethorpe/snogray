// image-output.h -- High-level image output
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_OUTPUT_H__
#define __IMAGE_OUTPUT_H__

#include <memory>
#include <string>

#include "filter-conv.h"
#include "image-io.h"


namespace snogray {

class ImageOutput
{
public:

  // Basically an ImageRow, but with sample weighting information.
  //
  struct SampleRow
  {
    SampleRow (unsigned width = 0) : pixels (width), weights (width) { }

    void resize (unsigned width)
    {
      pixels.resize (width, Tint (0, 0));
      weights.resize (width, 0);
    }

    void clear ()
    {
      pixels.clear ();
      weights.assign (weights.size(), 0);
    }

    ImageRow pixels;
    std::vector<float> weights;
  };

  ImageOutput (const std::string &filename, unsigned _width, unsigned _height,
	       const ValTable &params = ValTable::NONE);
  ~ImageOutput ();

  // Add a sample with value TINT at floating point position SX, SY.
  // TINT's contribution to adjacent pixels is determined by the
  // anti-aliasing filter in effect; if there is none, then it is basically
  // just added to the nearest pixel.  The floating-point center of a pixel
  // is at its integer coordinates + (0.5, 0.5).
  //
  void add_sample (float sx, float sy, const Tint &tint)
  {
    Tint clamped = tint;

    if (exposure != 0)
      clamped *= intensity_scale;
    if (intensity_power != 1)
      clamped = Tint (pow (clamped.unscaled_color(), intensity_power),
		      clamped.alpha);
    if (max_intens != 0)
      clamped = clamped.clamp (max_intens);

    filter_conv.add_sample (sx, sy, clamped, *this);
  }

  // Write the completed portion of the output image to disk, if possible.
  // This may flush I/O buffers etc., but will not in any way change the
  // output (so for instance, it will _not_ flush the compression state of
  // a PNG output image, as that can make the resulting compression worse).
  //
  void flush () { sink->flush (); }

  // Return true if the output has an alpha (opacity) channel.
  //
  bool has_alpha_channel () const { return sink->has_alpha_channel (); }

  // Make sure at least NUM rows are buffered in memory before being written.
  // NUM is a minimum -- more rows may be buffered if necessary to support
  // the output filter, or for other internal reasons.
  //
  void set_num_buffered_rows (unsigned num);

  // Flush any buffered rows until the current minimum (buffered) row is MIN_Y.
  //
  void set_min_y (int min_y);

  // Return the number rows or columns on either side of any pixel that are
  // effective when a sample is added inside that pixel (because of filter
  // convolution).
  //
  unsigned filter_radius () const { return filter_conv.filter_radius; }

  // Add a sample with value TINT at integer coordinates PX, PY.  WEIGHT
  // controls how much this sample counts relative to other samples added
  // at the same coordinates.  It is assumed that TINT has already been
  // scaled by WEIGHT.
  //
  // [This method is a callback used by Filterconv<ImageOutput>.]
  //
  void add_sample (int px, int py, const Tint &tint, float weight)
  {
    SampleRow &r = row (py);
    r.pixels[px] += tint;
    r.weights[px] += weight;
  }

  // Return true if the given X or Y coordinate is valid.
  //
  // [These methods are callbacks used by Filterconv<ImageOutput>.]
  //
  bool valid_x (int px) { return px >= 0 && px < int (width); }
  bool valid_y (int py) { return py >= min_y && py < int (height); }

  // Returns a row at absolute position Y.  Rows cannot be addressed
  // completely randomly, as only NUM_BUFFERED_ROWS rows are buffered in
  // memory; if a row which has already been output is specified, an error
  // is signaled.
  //
  SampleRow &row (int y)
  {
    if (y >= buf_y && y < buf_y + int (num_buffered_rows))
      return rows[y % num_buffered_rows];
    else
      return _row (y);
  }

  // Alias for row() method.
  //
  SampleRow &operator[] (int y) { return row (y); }

  // Size of output image.
  //
  unsigned width, height;

  // Lowest possible row (no output is ever done below this).
  //
  int min_y;

  // The intensity of the output image is scaled by 2^exposure.
  //
  float exposure;


private:

  // Write the the lowest currently buffered row to the output sink, and
  // recycle its storage for use by another row.  BUF_Y is incremented to
  // reflect the new lowest buffered row.
  //
  void flush_min_row ();

  // Internal version of the row() method which handles over-the-horizon cases.
  //
  SampleRow &_row (int y);

  // Where the output goes.
  //
  std::auto_ptr<ImageSink> sink;

  FilterConv<ImageOutput, Tint> filter_conv;

  // Number of rows kept buffered in memory.
  //
  unsigned num_buffered_rows;

  // Number of buffered rows specified by user.
  //
  unsigned num_user_buffered_rows;

  // Currently available rows (NUM_BUFFERED_ROWS).
  //
  std::vector<SampleRow> rows;

  // Lowest row currently buffered in memory.
  //
  int buf_y;

  float intensity_scale;   // intensity multiplier (2^exposure)
  float intensity_power;   // power which intensity is raised to (1 == nop)

  Color::component_t max_intens;
};

}

#endif /* __IMAGE_OUTPUT_H__ */

// arch-tag: 4e362922-5358-4423-80c0-2a3d3d8100fe
