// image-output.cc -- High-level image output
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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
#include "snogassert.h"
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
    intensity_scale (params.get_float ("exposure", 1)),
    intensity_power (params.get_float ("contrast", 1))
{
}

void
ImageOutput::_set_min_y (int new_min_y)
{
  ASSERT (new_min_y >= min_y);

  // Make sure there is no gap between the current bottom-most buffered
  // row (ImageOutput::min_y + ImageOutput::rows.size()), and NEW_MIN_Y.
  // This should really never happen, so we just add new rows which
  // will be immediately deleted aftering writing them below.
  //
  if (new_min_y > min_y + int (rows.size ()))
    _row (new_min_y - 1);

  // Write out any rows between the old and new values of min_y.
  //
  while (min_y < new_min_y)
    {
      SampleRow *r = rows.front ();

      rows.pop_front ();
      min_y++;

      for (unsigned x = 0; x < width; x++)
	{
	  Tint pixel = r->pixels[x];

	  float weight = r->weights[x];
	  if (weight > 0)
	    pixel /= weight;

	  if (intensity_scale != 1)
	    pixel *= intensity_scale;
	  if (intensity_power != 1)
	    pixel = Tint (pow (pixel.unscaled_color(), intensity_power),
			  pixel.alpha);

	  r->pixels[x] = pixel;
	}

      sink->write_row (r->pixels);

      delete r;
    }

  ASSERT (min_y == new_min_y);
}

ImageOutput::~ImageOutput ()
{
  // Write as-yet unwritten rows
  //
  _set_min_y (height);
  flush ();
}


// Low-level row handling

// Returns a row at absolute position Y.  Rows cannot be addressed
// completely randomly, as only NUM_BUFFERED_ROWS rows are buffered in
// memory; if a row which has already been output is specified, an error
// is signaled.
//
ImageOutput::SampleRow &
ImageOutput::_row (int y)
{
  ASSERT (y >= min_y);

  // Add new rows as necessary
  //
  while (y >= min_y + int (rows.size ()))
    rows.push_back (new SampleRow (width));

  return *rows[y - min_y];
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
  filter_conv.add_sample (sx, sy, tint, *this);
}


// arch-tag: b4e1bbd7-c070-4ac9-9075-b9abcaefc30a
