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

#include <cmath>

extern "C" {
#include <ppm.h>
}

#include "image-ppm.h"

using namespace Snogray;


// Output

class PpmImageSink : public ImageSink
{
public:

  PpmImageSink (const PpmImageSinkParams &params);
  ~PpmImageSink ();

  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  // Floating-point to pixval conversion
  pixval color_component_to_pixval (Color::component_t com)
  {
    if (com < 0)
      return 0;

    com = powf (com, 1 / IMAGE_PPM_GAMMA); // gamma correction

    if (com >= 1)
      return max_pixval;
    else
      return (pixval)(max_pixval * com);
  }

private:

  FILE *stream;

  // A single row of bytes we use as temporary storage during output
  //
  pixel *output_row;

  // PPM params (currently these have fixed values)
  //
  pixval max_pixval;
  bool force_plain;
};

PpmImageSink::PpmImageSink (const PpmImageSinkParams &params)
  : ImageSink (params),
    output_row (ppm_allocrow (params.width)),
    //
    // XXX need someway to let the user set these...
    //
    max_pixval (255), force_plain (false)
{
  // Open output file
  //
  stream = fopen (params.file_name, "wb");
  if (! stream)
    params.sys_error ("Could not open output file");

  // Write file header
  //
  ppm_writeppminit (stream, params.width, params.height,
		    max_pixval, force_plain);
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

float
PpmImageSink::max_intens () const
{
  return 1;
}

ImageSink *
PpmImageSinkParams::make_sink () const
{
  return new PpmImageSink (*this);
}


// Input

class PpmImageSource : public ImageSource
{  
public:
  PpmImageSource (const PpmImageSourceParams &params);
  ~PpmImageSource ();

  virtual void read_size (unsigned &width, unsigned &height);
  virtual void read_row (ImageRow &row);

  // Pixval to floating-point conversion
  Color::component_t pixval_to_color_component (pixval pv)
  {
    Color::component_t com = ((Color::component_t)pv) / max_pixval;

    com = powf (com, IMAGE_PPM_GAMMA); // undo gamma correction

    return com;
  }

private:

  FILE *stream;

  unsigned width, height;

  // A single row of bytes we use as temporary storage during input
  //
  pixel *input_row;

  // PPM params
  //
  pixval max_pixval;
  int format;
};

PpmImageSource::PpmImageSource (const PpmImageSourceParams &params)
{
  // Open input file
  //
  stream = fopen (params.file_name, "rb");
  if (! stream)
    params.sys_error ("Could not open input file");

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
PpmImageSource::read_size (unsigned &_width, unsigned &_height)
{
  _width = width;
  _height = height;
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

ImageSource *
PpmImageSourceParams::make_source () const
{
  return new PpmImageSource (*this);
}

// arch-tag: 1a5e77d9-3076-4197-80da-57643fe8b5af
