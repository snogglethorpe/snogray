// image-png.cc -- PNG format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstdio>

#include <libpng/png.h>

#include "image-png.h"

using namespace Snogray;

class PngImageSink : public RgbByteImageSink
{  
public:
  PngImageSink (unsigned width, unsigned height,
		const PngImageSinkParams &params);
  ~PngImageSink ();

  virtual void write_row (const unsigned char *rgb_bytes);

private:
  const char *file_name;

  FILE *stream;

  png_structp png;
  png_infop png_info;
};

PngImageSink::PngImageSink (unsigned width, unsigned height,
			    const PngImageSinkParams &params)
  : RgbByteImageSink (width, height, params),
    file_name (params.file_name)
{
  // Open output file

  stream = fopen (file_name, "wb");
  if (! stream)
    params.sys_error ("Could not open output file");

  // Create libpng data structures

  png = png_create_write_struct (PNG_LIBPNG_VER_STRING, (png_voidp)0, 0, 0);
  if (! png)
    {
      fclose (stream);
      params.sys_error ("Could not create PNG struct");
    }

  png_info = png_create_info_struct (png);
  if (! png_info)
    {
      png_destroy_write_struct(&png, (png_infopp)0);
      fclose (stream);
      params.sys_error ("Could not create PNG info struct");
    }

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      params.sys_error ("Error writing PNG file");
    }

  // Write file header

  png_set_IHDR (png, png_info, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_gAMA (png, png_info, gamma_correction);

  png_init_io (png, stream);

  png_write_info (png, png_info);
}

PngImageSink::~PngImageSink ()
{
  if (setjmp (png_jmpbuf (png)))
    {
      fprintf (stderr, "%s: Error destroying PNG object!\n", file_name);
      exit (97);
    }

  png_write_end (png, png_info);
  png_destroy_write_struct (&png, &png_info);

  fclose (stream);
}

void
PngImageSink::write_row (const unsigned char *rgb_bytes)
{
  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      fprintf (stderr, "%s: Error writing PNG file\n", file_name);
      exit (96);
    }

  png_write_row (png, (png_byte *)rgb_bytes);

  // Flushing every line screws up compression; the docs say that doing
  // so periodically but less often (e.g., `using png_set_flush') works
  // better, but doesn't specify how often is good.
  //////png_write_flush (png);
}

ImageSink *
PngImageSinkParams::make_sink (unsigned width, unsigned height) const
{
  return new PngImageSink (width, height, *this);
}

// arch-tag: 034a86a6-eef0-47ad-9a04-17074e04e62e
