// image-byte-vec.cc -- Common code for image formats based on vectors of bytes
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
#include "string-funs.h"

#include "image-byte-vec.h"

using namespace snogray;


// Output

ByteVecImageSink::ByteVecImageSink (const std::string &filename,
				    unsigned width, unsigned height,
				    const ValTable &params)
  : ImageSink (filename, width, height, params),
    target_gamma (params.get_float ("gamma", DEFAULT_TARGET_GAMMA)),
    gamma_correction (1 / target_gamma),
    output_row (width * 3)
{ }

void
ByteVecImageSink::write_row (const ImageRow &row)
{
  unsigned width = row.width;
  ByteVec::iterator p = output_row.begin ();

  for (unsigned x = 0; x < width; x++)
    {
      const Color &col = row[x];
      *p++ = color_component_to_byte (col.r ());
      *p++ = color_component_to_byte (col.g ());
      *p++ = color_component_to_byte (col.b ());
    }

  write_row (output_row);
}

float
ByteVecImageSink::max_intens () const
{
  return 1;
}


// Input

ByteVecImageSource::ByteVecImageSource (const std::string &filename,
					const ValTable &params)
  : ImageSource (filename, params),
    gamma_correction (params.get_float ("gamma", DEFAULT_SOURCE_GAMMA)),
    //
    // The following must be set by subclass using `set_size' after
    // reading image header
    //
    input_row (0), bytes_per_component (1), component_scale (1 / 255.0),
    num_channels (3)
{ }


// Called by subclass (usually after reading image header) to finish
// setting up stuff.
//
void
ByteVecImageSource::set_specs (unsigned _width, unsigned _height,
			       unsigned _num_channels, unsigned bit_depth)
{
  width = _width;
  height = _height;
  num_channels = _num_channels;

  // Make sure bit-depth is rational: <= 16, and a power of two
  //
  if (bit_depth <= 0 || bit_depth > 16 || (bit_depth & (bit_depth - 1)))
    throw bad_format (std::string ("unsupported bit depth: ")
		      + stringify (bit_depth));

  // Make sure it's a number of channels we support
  //
  if (_num_channels < 1 || _num_channels > 4)
    throw bad_format (std::string ("unsupported number of channels: ")
		      + stringify (_num_channels));

  // We allocate either one or two bytes per pixel per channel
  // [we don't _really_ support sub-byte bit-depeths -- we rely on
  // subclasses to unpack those into bytes where needed]
  //
  bytes_per_component = bit_depth <= 8 ? 1 : 2;

  component_scale = 1 / Color::component_t ((1 << bit_depth) - 1);

  input_row.resize (_width * _num_channels * bytes_per_component);
}

void
ByteVecImageSource::read_row (ImageRow &row)
{
  read_row (input_row);

  unsigned width = row.width;
  ByteVec::const_iterator p = input_row.begin ();

  for (unsigned x = 0; x < width; x++)
    {
      Color::component_t r, g, b, a;

      r = next_color_component (p);

      if (num_channels >= 3)
	{
	  g = next_color_component (p);
	  b = next_color_component (p);
	}
      else
	g = b = r;

      if (num_channels == 2 || num_channels == 4)
	a = next_alpha_component (p);
      else
	a = 1;

      row[x] = Color (r, g, b);	// alpha????
    }
}


// arch-tag: ee0370d1-7cdb-42d4-96e3-4cf7757cc2cf
