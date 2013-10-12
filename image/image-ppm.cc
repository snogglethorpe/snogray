// image-ppm.cc -- PPM/PGM/PBM format image handling
//
//  Copyright (C) 2005-2008, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/snogmath.h"
#include "util/string-funs.h"

#include "image-ppm.h"

using namespace snogray;



// ----------------------------------------------------------------
// PpmImageSink: PPM image input


PpmImageSink::PpmImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const ValTable &params)
  : ByteVecImageSink (filename, width, height, params),
    output_row (ppm_allocrow (width))
{
  // Check gamma.
  //
  if (abs (target_gamma - float (IMAGE_PPM_GAMMA)) > 0.01f)
    open_err ("PPM format uses a fixed gamma of " _IMAGE_PPM_GAMMA_STRING);

  // Check component size.
  //
  max_pixval = (1 << bits_per_component) - 1;
  if (max_pixval > PPM_MAXMAXVAL)
    open_err ("maximum component value ("
	      + stringify (max_pixval)
	      + ") greater than PPM format allows ("
	      + stringify (PPM_MAXMAXVAL) + ")");

  // Open output file
  //
  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ("", true);

  // Write file header
  //
  ppm_writeppminit (stream, width, height, max_pixval, false /* force_plain */);
}

PpmImageSink::~PpmImageSink ()
{
  ppm_freerow (output_row);
}


// Note that we override the ImageSink::read_row(ImageRow&), instead
// of ByteVecImageSink::read_row(const ByteVec&) as is normal for
// subclasses of ByteVecImageSink.  This because PPM has its own
// abstraction for writing into an image row; however we still use
// other facilities of ByteVecImageSink, such as float-to-integer
// component conversion, etc.
//
void
PpmImageSink::write_row (const ImageRow &row)
{
  unsigned width = row.width;

  for (unsigned x = 0; x < width; x++)
    {
      const Color &col = row[x].alpha_scaled_color ();

      PPM_ASSIGN (output_row[x],
		  color_component_to_int (col.r()),
		  color_component_to_int (col.g()),
		  color_component_to_int (col.b()));
    }

  ppm_writeppmrow (stream, output_row, width, max_pixval, false/*force_plain*/);
}



// ----------------------------------------------------------------
// PpmImageSource: PPM image output


PpmImageSource::PpmImageSource (const std::string &filename,
				const ValTable &params)
  : ByteVecImageSource (filename, params)
{
  // Open input file
  //
  stream = fopen (filename.c_str(), "rb");
  if (! stream)
    open_err ("", true);

  // Read file header
  //
  int _width, _height;
  ppm_readppminit (stream, &_width, &_height, &max_pixval, &format);

  // Compute bits-per-component from MAX_PIXVAL; we can only handle a
  // power-of-two MAX_PIXVAL.
  //
  unsigned comp_bound = 1;
  unsigned comp_bits = 0;
  while (comp_bound < max_pixval + 1)
    {
      comp_bits++;
      comp_bound <<= 1;
    }
  if (comp_bound != max_pixval + 1)
    open_err ("PPM image has an unsupported MAX_PIXVAL ("
	      + stringify (max_pixval) + ")");

  set_specs (_width, _height, PIXEL_FORMAT_RGB, 8 * comp_bits, comp_bits);

  // Alloc temp row buffer
  //
  input_row = ppm_allocrow (width);
}

PpmImageSource::~PpmImageSource ()
{
  ppm_freerow (input_row);
}


// Note that we override the ImageSource::read_row(ImageRow&),
// instead of ByteVecImageSource::read_row(ByteVec&) as is normal
// for subclasses of ByteVecImageSource.  This because PPM has its
// own abstraction for reading from an image row; however we still
// use other facilities of ByteVecImageSource, such as int-to-float
// component conversion, etc.
//
void
PpmImageSource::read_row (ImageRow &row)
{
  ppm_readppmrow (stream, input_row, width, max_pixval, format);

  for (unsigned x = 0; x < width; x++)
    {
      component_t r = int_to_color_component (PPM_GETR (input_row[x]));
      component_t g = int_to_color_component (PPM_GETG (input_row[x]));
      component_t b = int_to_color_component (PPM_GETB (input_row[x]));

      row[x].set_rgb (r, g, b);
    }
}


// arch-tag: 1a5e77d9-3076-4197-80da-57643fe8b5af
