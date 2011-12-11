// image-output.h -- High-level image output
//
//  Copyright (C) 2005-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_OUTPUT_H
#define SNOGRAY_IMAGE_OUTPUT_H

#include <string>
#include <vector>
#include <deque>

#include "unique-ptr.h"
#include "image-filter-conv.h"
#include "image-io.h"


namespace snogray {


class ImageOutput
{
public:

  // Basically an ImageRow, but with sample weighting information.
  //
  struct SampleRow
  {
    SampleRow (unsigned width) : pixels (width), weights (width) { }

    void clear ()
    {
      pixels.clear ();
      weights.assign (weights.size(), 0);
    }

    ImageRow pixels;
    std::vector<float> weights;
  };

  // Create an ImageOutput object for writing to FILENAME, with a size
  // of WIDTH, HEIGHT.  PARAMS holds any additional optional parameters.
  //
  ImageOutput (const std::string &filename,
	       unsigned width, unsigned height,
	       const ValTable &params = ValTable::NONE);
  ~ImageOutput ();

  // Add a sample with value TINT at floating point position SX, SY.
  // TINT's contribution to adjacent pixels is determined by the
  // anti-aliasing filter in effect; if there is none, then it is basically
  // just added to the nearest pixel.  The floating-point center of a pixel
  // is at its integer coordinates + (0.5, 0.5).
  //
  void add_sample (float sx, float sy, const Tint &tint);

  // Write the completed portion of the output image to disk, if possible.
  // This may flush I/O buffers etc., but will not in any way change the
  // output (so for instance, it will _not_ flush the compression state of
  // a PNG output image, as that can make the resulting compression worse).
  //
  void flush () { sink->flush (); }

  // Return true if the output has an alpha (opacity) channel.
  //
  bool has_alpha_channel () const { return sink->has_alpha_channel (); }

  // Flush any buffered rows until the current minimum (buffered) row is
  // ImageOutput::min_y.  NEW_MIN_Y is in the sample coordinate-system,
  // not the output coordinate-system.
  //
  void set_min_sample_y (int new_min_y)
  {
    // Set the raw min_y leaving some room for the filter support,
    // and converting between the sample coordinate-system and the
    // output-image coordinate-system.
    //
    new_min_y -= int (sample_base_y);
    new_min_y -= int (filter_y_radius ());
    set_raw_min_y (max (min_y, new_min_y));
  }

  // Flush any buffered rows until the current minimum (buffered) row is
  // ImageOutput::min_y.  Unlike ImageOutput::set_min_sample_y, this
  // directly operates on the buffer, in the coordinate-system of the
  // output image, and does not add any adjustment for the filter
  // support or for any offset between the sample and output-image
  // coordinate-systems.
  //
  void set_raw_min_y (int new_min_y);

  // Return the number of columns/rows on either side of any pixel
  // that are effective when a sample is added inside that pixel
  // (because of filter convolution).
  //
  unsigned filter_x_radius () const { return filter_conv.filter_x_radius; }
  unsigned filter_y_radius () const { return filter_conv.filter_y_radius; }

  // Add a sample with value TINT at integer coordinates PX, PY.  WEIGHT
  // controls how much this sample counts relative to other samples added
  // at the same coordinates.  It is assumed that TINT has already been
  // scaled by WEIGHT.
  //
  // [This method is a callback used by ImageFilterConv<ImageOutput>.]
  //
  void add_sample (int px, int py, const Tint &tint, float weight)
  {
    SampleRow &r = row (py);
    r.pixels[px] += tint;
    r.weights[px] += weight;
  }

  // Return true if the given X or Y coordinate is valid.
  // The coordinates are in the output image's coordinate-system
  // (so in the range 0,0 - WIDTH,HEIGHT).
  //
  // [These methods are callbacks used by ImageFilterConv<ImageOutput>.]
  //
  bool valid_x (int px) { return px >= 0 && px < int (width); }
  bool valid_y (int py) { return py >= min_y && py < int (height); }

  // Returns a row at absolute position Y.  Rows cannot be addressed
  // completely randomly, as only rows above ImageOutput::min_y are
  // buffered in memory; if a row less than ImageOutput::min_y is
  // specified, an error is signaled.
  //
  SampleRow &row (int y)
  {
    int offs = y - min_y;
    if (offs >= 0 && offs < int (rows.size ()))
      return *rows[offs];
    else
      return _row (y);
  }

  // Alias for row() method.
  //
  SampleRow &operator[] (int y) { return row (y); }

  // Size of output image.
  //
  unsigned width, height;

  // Modifiers applied to output values.  Note that these are applied
  // when a particular row is flushed to the output file, not while the
  // row is being accumulated (this usually doesn't matter, but can in
  // the case of image recovery, where previous _output_ values are
  // being copied).
  //
  float intensity_scale;   // intensity multiplier (1 == nop)
  float intensity_power;   // power which intensity is raised to (1 == nop)

private:

  // Row number of first row buffered in memory.  No row before this can
  // be addressed.
  //
  int min_y;

  // Base-coordinates of the sample coordinate-system.
  //
  float sample_base_x, sample_base_y;

  // Internal version of the ImageOutput::row() method which handles
  // rows not in ImageOutput::rows.
  //
  SampleRow &_row (int y);

  // Where the output goes.
  //
  UniquePtr<ImageSink> sink;

  ImageFilterConv<ImageOutput, Tint> filter_conv;

  // Currently available rows.  The row number of the first row is
  // ImageOutput::min_y.
  //
  std::deque<SampleRow *> rows;
};


}

#endif // SNOGRAY_IMAGE_OUTPUT_H


// arch-tag: 4e362922-5358-4423-80c0-2a3d3d8100fe
