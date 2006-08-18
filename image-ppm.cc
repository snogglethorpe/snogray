// image-ppm.cc -- PPM/PGM/PBM format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"

#include "image-ppm.h"

using namespace Snogray;


// Output

PpmImageSink::PpmImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const Params &params)
  : ImageSink (filename, width, height, params),
    output_row (ppm_allocrow (width)),
    //
    // XXX need someway to let the user set these...
    //
    max_pixval (255), force_plain (false)
{
  float gamma = params.get_float ("gamma", IMAGE_PPM_GAMMA);

  if (gamma < IMAGE_PPM_GAMMA - 0.01 || gamma > IMAGE_PPM_GAMMA + 0.01)
    open_err ("PPM format uses a fixed gamma of " _IMAGE_PPM_GAMMA_STRING);

  // Open output file
  //
  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ("", true);

  // Write file header
  //
  ppm_writeppminit (stream, width, height, max_pixval, force_plain);
}

PpmImageSink::~PpmImageSink ()
{
  ppm_freerow (output_row);
}

void
PpmImageSink::write_row (const ImageRow &row)
{
  unsigned width = row.width;

  for (unsigned x = 0; x < width; x++)
    {
      const Color &col = row[x];
      PPM_ASSIGN (output_row[x],
		  color_component_to_pixval (col.r()),
		  color_component_to_pixval (col.g()),
		  color_component_to_pixval (col.b()));
    }

  ppm_writeppmrow (stream, output_row, width, max_pixval, force_plain);
}


// Input

PpmImageSource::PpmImageSource (const std::string &filename,
				const Params &params)
  : ImageSource (filename, params)
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

  // Convert int -> unsigned
  //
  width = _width;
  height = _height;

  // Alloc temp row buffer
  //
  input_row = ppm_allocrow (width);
}

PpmImageSource::~PpmImageSource ()
{
  ppm_freerow (input_row);
}

void
PpmImageSource::read_row (ImageRow &row)
{
  ppm_readppmrow (stream, input_row, width, max_pixval, format);

  for (unsigned x = 0; x < width; x++)
    {
      Color::component_t r
	= pixval_to_color_component (PPM_GETR (input_row[x]));
      Color::component_t g
	= pixval_to_color_component (PPM_GETG (input_row[x]));
      Color::component_t b
	= pixval_to_color_component (PPM_GETB (input_row[x]));

      row[x].set_rgb (r, g, b);
    }
}


// arch-tag: 1a5e77d9-3076-4197-80da-57643fe8b5af
