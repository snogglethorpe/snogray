// image-png.cc -- PNG format image handling
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

#include "util/excepts.h"

#include "image-png.h"

using namespace snogray;


// Common code for libpng access

void
PngErrState::throw_libpng_err ()
{
  if (libpng_err)
    throw std::runtime_error (err_filename + ": " + libpng_err_msg);
}

void
PngErrState::libpng_err_handler (png_structp libpng_struct, const char *msg)
{
  PngErrState *state
    = static_cast<PngErrState *> (png_get_error_ptr (libpng_struct));

  if (! state->libpng_err)
    {
      state->libpng_err = true;
      state->libpng_err_msg = msg;
    }

  longjmp (png_jmpbuf (libpng_struct), 1);
}


// Output

PngImageSink::PngImageSink (const std::string &_filename,
			    unsigned width, unsigned height,
			    const ValTable &params)
  : ByteVecImageSink (_filename, width, height, params),
    PngErrState (filename)
{
  volatile unsigned color_type;	// volatile because we use setjmp below
  switch (pixel_format)
    {
    case PIXEL_FORMAT_GREY:
      color_type = PNG_COLOR_TYPE_GRAY; break;
    case PIXEL_FORMAT_GREY_ALPHA:
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
    case PIXEL_FORMAT_RGB:
      color_type = PNG_COLOR_TYPE_RGB; break;
    case PIXEL_FORMAT_RGBA:
      color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
    default:
      open_err ("unsupported pixel-format: " + pixel_format_name);
    }

  // Open output file
  //
  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ("", true);

  // Create libpng data structures

  libpng_struct
    = png_create_write_struct (PNG_LIBPNG_VER_STRING,
			       png_voidp (static_cast<PngErrState *>(this)),
			       libpng_err_handler, 0);
  if (! libpng_struct)
    {
      fclose (stream);
      open_err ("Could not create PNG struct");
    }

  libpng_info = png_create_info_struct (libpng_struct);
  if (! libpng_info)
    {
      png_destroy_write_struct(&libpng_struct, 0);
      fclose (stream);
      open_err ("Could not create PNG info struct");
    }

  if (setjmp (png_jmpbuf (libpng_struct)))
    {
      png_destroy_write_struct (&libpng_struct, &libpng_info);
      fclose (stream);
      throw_libpng_err ();
    }

  // Write file header

  png_set_IHDR (libpng_struct, libpng_info, width, height,
		bytes_per_component * 8, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_gAMA (libpng_struct, libpng_info, gamma_correction);

  png_init_io (libpng_struct, stream);

  png_write_info (libpng_struct, libpng_info);
}

PngImageSink::~PngImageSink ()
{
  if (!libpng_err && setjmp (png_jmpbuf (libpng_struct)) == 0)
    png_write_end (libpng_struct, libpng_info);

  png_destroy_write_struct (&libpng_struct, &libpng_info);

  fclose (stream);

  if (libpng_err)
    throw_libpng_err ();
}

void
PngImageSink::write_row (const ByteVec &byte_vec)
{
  if (!libpng_err && setjmp (png_jmpbuf (libpng_struct)) == 0)
    png_write_row (libpng_struct, const_cast<png_byte *> (&byte_vec[0]));
  else
    throw_libpng_err ();
}

// Write previously written rows to disk, if possible.  This may flush
// I/O buffers etc., but will not in any way change the output (so for
// instance, it will _not_ flush the compression state of a PNG output
// image, as that can make the resulting compression worse).
//
void
PngImageSink::flush ()
{
  // Flushing every line screws up compression; the docs say that doing
  // so periodically but less often (e.g., `using png_set_flush') works
  // better, but doesn't specify how often is good.
  //////png_write_flush (libpng_struct);

  fflush (stream);
}


// Input

PngImageSource::PngImageSource (const std::string &_filename,
				const ValTable &params)
  : ByteVecImageSource (_filename, params),
    PngErrState (filename)
{
  // Open input file

  stream = fopen (filename.c_str(), "rb");
  if (! stream)
    open_err ("Could not open input file");

  // Create libpng data structures

  libpng_struct
    = png_create_read_struct (PNG_LIBPNG_VER_STRING,
			      png_voidp (static_cast<PngErrState *>(this)),
			      libpng_err_handler, 0);
  if (! libpng_struct)
    {
      fclose (stream);
      open_err ("Could not create PNG struct");
    }

  libpng_info = png_create_info_struct (libpng_struct);
  if (! libpng_info)
    {
      png_destroy_read_struct (&libpng_struct, 0, 0);
      fclose (stream);
      open_err ("Could not create PNG info struct");
    }

  if (setjmp (png_jmpbuf (libpng_struct)))
    {
      png_destroy_read_struct (&libpng_struct, &libpng_info, 0);
      fclose (stream);
      throw_libpng_err ();
    }

  // Read file header

  png_init_io (libpng_struct, stream);

  png_read_info (libpng_struct, libpng_info);

  png_uint_32 _width, _height;
  int _bit_depth;
  int color_type, interlace_method, compression_method, filter_method;
  png_get_IHDR (libpng_struct, libpng_info,
		&_width, &_height, &_bit_depth, &color_type,
		&interlace_method, &compression_method, &filter_method);

  double encoding_gamma_correction;
  if (png_get_gAMA (libpng_struct, libpng_info, &encoding_gamma_correction))
    set_target_gamma (1 / encoding_gamma_correction);

  PixelFormat pxfmt;
  switch (color_type)
    {
    case PNG_COLOR_TYPE_GRAY:	    pxfmt = PIXEL_FORMAT_GREY; break;
    case PNG_COLOR_TYPE_GRAY_ALPHA: pxfmt = PIXEL_FORMAT_GREY_ALPHA; break;
    case PNG_COLOR_TYPE_RGB:	    pxfmt = PIXEL_FORMAT_RGB; break;
    case PNG_COLOR_TYPE_RGB_ALPHA:  pxfmt = PIXEL_FORMAT_RGBA; break;

    case PNG_COLOR_TYPE_PALETTE:
      png_set_palette_to_rgb (libpng_struct);
      pxfmt = PIXEL_FORMAT_RGB;
      break;

    default:
      png_destroy_read_struct (&libpng_struct, &libpng_info, 0);
      fclose (stream);
      open_err ("Unsupported PNG image type");
    }

  // Expand sub-byte grey-scale bit-depths to one-byte-per-pxel
  //
  if (color_type == PNG_COLOR_TYPE_GRAY && _bit_depth < 8)
#if HAVE_PNG_SET_EXPAND_GRAY_1_2_4_TO_8
    png_set_expand_gray_1_2_4_to_8 (libpng_struct);
#else
    png_set_gray_1_2_4_to_8 (libpng_struct);
#endif

  // Convert a tRNS chunk to a full alpha channel.
  //
  if (png_get_valid (libpng_struct, libpng_info, PNG_INFO_tRNS))
    {
      png_set_tRNS_to_alpha (libpng_struct);
      pxfmt = pixel_format_add_alpha_channel (pxfmt);
    }

  // We allocate either one or two bytes per pixel per channel
  //
  unsigned _bytes_per_component = (_bit_depth <= 8) ? 1 : 2;

  set_specs (_width, _height, pxfmt, _bytes_per_component);
}

PngImageSource::~PngImageSource ()
{
  if (!libpng_err && setjmp (png_jmpbuf (libpng_struct)) == 0)
    png_read_end (libpng_struct, 0);

  png_destroy_read_struct (&libpng_struct, &libpng_info, 0);

  fclose (stream);

  if (libpng_err)
    throw_libpng_err ();
}

void
PngImageSource::read_row (ByteVec &byte_vec)
{
  if (!libpng_err && setjmp (png_jmpbuf (libpng_struct)) == 0)
    png_read_row (libpng_struct, static_cast<png_byte *>(&byte_vec[0]), 0);
  else
    throw_libpng_err ();
}


// arch-tag: 034a86a6-eef0-47ad-9a04-17074e04e62e
