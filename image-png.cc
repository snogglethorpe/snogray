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
#include <cmath>

#include <libpng/png.h>

#include "image-png.h"

using namespace Snogray;

class PngImageSink : public ImageSink
{  
public:
  PngImageSink (const PngImageSinkParams &params);
  ~PngImageSink ();

  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  float gamma_correction;

private:
  unsigned char color_component_to_byte (Color::component_t com)
  {
    if (com < 0)
      return 0;

    if (gamma_correction != 0.0)
      com = powf (com, gamma_correction);

    if (com >= 0.9999)
      return 255;
    else
      return (unsigned char)(256.0 * com);
  }

  const char *file_name;
  unsigned width;
  FILE *stream;
  png_structp png;
  png_infop png_info;
  unsigned char *output_bytes;
};

PngImageSink::PngImageSink (const PngImageSinkParams &params)
  : file_name (params.file_name), width (params.width),
    output_bytes (new unsigned char[params.width * 3])
{
  float targ_gamma = params.target_gamma;

  if (targ_gamma == 0)
    targ_gamma = ImageSinkParams::DEFAULT_TARGET_GAMMA;

  gamma_correction = 1 / targ_gamma;

  stream = fopen (file_name, "wb");
  if (! stream)
    {
      fprintf (stderr, "%s: Could not open output file: %m\n", file_name);
      exit (99);
    }

  png = png_create_write_struct (PNG_LIBPNG_VER_STRING, (png_voidp)0, 0, 0);
  if (! png)
    {
      fprintf (stderr, "%s: Could not create PNG struct\n", file_name);
      fclose (stream);
      exit (98);
    }

  png_info = png_create_info_struct (png);
  if (! png_info)
    {
      png_destroy_write_struct(&png, (png_infopp)0);
      fclose (stream);
      fprintf (stderr, "%s: Could not create PNG info struct\n", file_name);
      exit (97);
    }

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      fprintf (stderr, "%s: Error writing PNG file\n", file_name);
      exit (96);
    }

  png_set_IHDR (png, png_info, params.width, params.height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_gAMA (png, png_info, targ_gamma);

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
PngImageSink::write_row (const ImageRow &row)
{
  unsigned char *p = output_bytes;

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      fprintf (stderr, "%s: Error writing PNG file\n", file_name);
      exit (96);
    }

  for (unsigned x = 0; x < width; x++)
    {
      const Color &col = row[x];
      *p++ = color_component_to_byte (col.red);
      *p++ = color_component_to_byte (col.green);
      *p++ = color_component_to_byte (col.blue);
    }

  png_write_row (png, output_bytes);

  //png_write_flush (png);
}

float
PngImageSink::max_intens () const
{
  return 1;
}



ImageSink *
PngImageSinkParams::make_sink () const
{
  return new PngImageSink (*this);
}

// arch-tag: 034a86a6-eef0-47ad-9a04-17074e04e62e
