// image-output.cc -- High-level image output
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

#include <string>

#include "snogmath.h"
#include "excepts.h"
#include "filter.h"
#include "mitchell-filt.h"
#include "gauss-filt.h"
#include "box-filt.h"

#include "image-output.h"


using namespace snogray;



ImageOutput::ImageOutput (const std::string &filename,
			  unsigned _width, unsigned _height,
			  const ValTable &params)
  : width (_width), height (_height),
    min_y (0),
    sink (ImageSink::open (filename, _width, _height, params)),
    filter_conv (params),
    num_buffered_rows (filter_conv.filter_radius * 2 + 1),
    num_user_buffered_rows (0),
    rows (num_buffered_rows), buf_y (0),
    intensity_scale (params.get_float ("exposure", 1)),
    intensity_power (params.get_float ("contrast", 1))
{
  for (unsigned i = 0; i < rows.size (); i++)
    rows[i].resize (width);
}

// Write the the lowest currently buffered row to the output sink, and
// recycle its storage for use by another row.  BUF_Y is incremented
// to reflect the new lowest buffered row.
//
void
ImageOutput::flush_min_row ()
{
  SampleRow &r = rows[buf_y % num_buffered_rows];

  for (unsigned x = 0; x < width; x++)
    {
      float weight = r.weights[x];
      if (weight > 0)
	r.pixels[x] /= weight;
    }

  sink->write_row (r.pixels);

  r.clear ();

  buf_y++;
}

void
ImageOutput::set_min_y (int new_min_y)
{
  while (buf_y < new_min_y)
    flush_min_row ();
  min_y = new_min_y;
}

ImageOutput::~ImageOutput ()
{
  // Write as-yet unwritten rows
  //
  set_min_y (height);
  flush ();
}


// Low-level row handling

// Make sure at least NUM rows are buffered in memory before being
// written.  NUM is a minimum -- more rows may be buffered if necessary
// to support the output filter, or for other internal reasons.
//
void
ImageOutput::set_num_buffered_rows (unsigned num)
{
  // Growing
  //
  while (num > num_user_buffered_rows)
    {
      rows.push_back (SampleRow (width));
      num_user_buffered_rows++;
      num_buffered_rows++;
    }

  // We ignore requests to shrink the number of buffered rows
}

// Returns a row at absolute position Y.  Rows cannot be addressed
// completely randomly, as only NUM_BUFFERED_ROWS rows are buffered in
// memory; if a row which has already been output is specified, an error
// is signaled.
//
ImageOutput::SampleRow &
ImageOutput::_row (int y)
{
  if (y < buf_y)
    throw std::runtime_error ("Previously output image row addressed");

  while (int (buf_y + num_buffered_rows) <= y)
    flush_min_row ();

  return rows[y % num_buffered_rows];
}


// ImageOutput::add_sample

// Add a sample with value TINT at floating point position SX, SY.
// TINT's contribution to adjacent pixels is determined by the
// anti-aliasing filter in effect; if there is none, then it is basically
// just added to the nearest pixel.  The floating-point center of a pixel
// is at its integer coordinates + (0.5, 0.5).
//
void ImageOutput::add_sample (float sx, float sy, const Tint &tint)
{
  Tint clamped = tint;

  if (intensity_scale != 1)
    clamped *= intensity_scale;
  if (intensity_power != 1)
    clamped = Tint (pow (clamped.unscaled_color(), intensity_power),
		    clamped.alpha);

  filter_conv.add_sample (sx, sy, clamped, *this);
}


// arch-tag: b4e1bbd7-c070-4ac9-9075-b9abcaefc30a
