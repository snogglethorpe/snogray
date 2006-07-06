// image-output.h -- High-level image output
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_OUTPUT_H__
#define __IMAGE_OUTPUT_H__

#include <memory>
#include <string>

#include "image-io.h"


namespace Snogray {

class Filter;

class ImageOutput
{
public:

  static const float DEFAULT_NEG_CLAMP = -0.1;
  static const float MAX_FILTER_WIDTH_SCALE = 2.f;

  // Basically an ImageRow, but with sample weighting information.
  //
  struct SampleRow
  {
    SampleRow (unsigned width = 0) : pixels (width), weights (width) { }

    void resize (unsigned width)
    {
      pixels.resize (width);
      weights.resize (width);
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
	       const Params &params = Params::NONE);
  ~ImageOutput ();

  // Add a sample with value COLOR at floating point position SX, SY.
  // COLOR's contribution to adjacent pixels is determined by the
  // anti-aliasing filter in effect; if there is none, then it is basically
  // just added to the nearest pixel.  The floating-point center of a pixel
  // is at its integer coordinates + (0.5, 0.5).
  //
  void add_sample (float sx, float sy, const Color &color);

  // Returns a row at absolute position Y.  Rows cannot be addressed
  // completely randomly, as only NUM_BUFFERED_ROWS rows are buffered in
  // memory; if a row which has already been output is specified, an error
  // is signaled.
  //
  SampleRow &row (int y)
  {
    if (y >= cur_y_min && y < cur_y_min + int (num_buffered_rows))
      return rows[y % num_buffered_rows];
    else
      return _row (y);
  }

  // Alias for row() method.
  //
  SampleRow &operator[] (int y) { return row (y); }

  // Write the completed portion of the output image to disk, if possible.
  // This may flush I/O buffers etc., but will not in any way change the
  // output (so for instance, it will _not_ flush the compression state of
  // a PNG output image, as that can make the resulting compression worse).
  //
  void flush () { sink->flush (); }

  // Make sure at least NUM rows are buffered in memory before being written.
  // NUM is a minimum -- more rows may be buffered if necessary to support
  // the output filter, or for other internal reasons.
  //
  void set_num_buffered_rows (unsigned num);

  // Size of output image.
  //
  unsigned width, height;

  // Anti-aliasing filter.
  //
  const Filter *filter;

  // "Radius" of FILTER.  This is an integer defining the number of
  // adjacent pixels on all sides of a pixel which are effected by output
  // samples within it.  It is calculated by expanding the maximum
  // filter width to a pixel boundary, and subtracting one to eliminate
  // the center pixel.
  //
  int filter_radius;		// really unsigned, but g++ goes nuts with
				// warnings if we actually use that type

  // The intensity of the output image is scaled by 2^exposure.
  //
  float exposure;

  // A clamp for the minimum negative value of filtered sample points.
  // See the comment in ImageOutput::add_sample for more details.
  //
  float neg_clamp;


private:

  static Filter *make_filter (const Params &params);

  // Write the the lowest currently buffered row to the output sink, and
  // recycle its storage for use by another row.  CUR_Y_MIN is
  // incremented to reflect the new lowest buffered row.
  //
  void flush_min_row ();

  // Internal version of the row() method which handles over-the-horizon cases.
  //
  SampleRow &_row (int y);

  // Where the output goes.
  //
  std::auto_ptr<ImageSink> sink;

  // Number of rows kept buffered in memory.
  //
  unsigned num_buffered_rows;

  // Number of buffered rows specified by user.
  //
  unsigned num_user_buffered_rows;

  // Currently available rows (NUM_BUFFERED_ROWS).
  //
  std::vector<SampleRow> rows;

  // Lowest row currently in memory.
  //
  int cur_y_min;

  float intensity_scale;	// 2^exposure

  Color::component_t max_intens;
};

}

#endif /* __IMAGE_OUTPUT_H__ */

// arch-tag: 4e362922-5358-4423-80c0-2a3d3d8100fe
