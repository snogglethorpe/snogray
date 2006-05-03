// image-png.cc -- PNG format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"

#include "image-png.h"

using namespace Snogray;



// Output

PngImageSink::PngImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const Params &params)
  : ByteVecImageSink (filename, width, height, params)
{
  // Open output file

  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ("", true);

  // Create libpng data structures

  png = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (! png)
    {
      fclose (stream);
      open_err ("Could not create PNG struct");
    }

  png_info = png_create_info_struct (png);
  if (! png_info)
    {
      png_destroy_write_struct(&png, 0);
      fclose (stream);
      open_err ("Could not create PNG info struct");
    }

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      open_err ("Error writing PNG file");
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
    throw std::runtime_error (filename + ": Error destroying PNG object!");

  png_write_end (png, png_info);
  png_destroy_write_struct (&png, &png_info);

  fclose (stream);
}

void
PngImageSink::write_row (const ByteVec &byte_vec)
{
  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      throw std::runtime_error (filename + ": Error writing PNG file");
    }

  png_write_row (png, (png_byte *)(&byte_vec[0]));

  // Flushing every line screws up compression; the docs say that doing
  // so periodically but less often (e.g., `using png_set_flush') works
  // better, but doesn't specify how often is good.
  //////png_write_flush (png);
}


// Input

PngImageSource::PngImageSource (const std::string &filename,
				const Params &params)
  : ByteVecImageSource (filename, params)
{
  // Open input file

  stream = fopen (filename.c_str(), "rb");
  if (! stream)
    open_err ("Could not open input file");

  // Create libpng data structures

  png = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (! png)
    {
      fclose (stream);
      open_err ("Could not create PNG struct");
    }

  png_info = png_create_info_struct (png);
  if (! png_info)
    {
      png_destroy_read_struct(&png, 0, 0);
      fclose (stream);
      open_err ("Could not create PNG info struct");
    }

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_read_struct (&png, &png_info, 0);
      fclose (stream);
      open_err ("Error writing PNG file");
    }

  // Read file header

  png_init_io (png, stream);

  png_read_info (png, png_info);

  png_uint_32 _width, _height;
  int _bit_depth;
  int color_type, interlace_method, compression_method, filter_method;
  png_get_IHDR (png, png_info, &_width, &_height, &_bit_depth, &color_type,
		&interlace_method, &compression_method, &filter_method);

  double _gamma_correction;
  png_get_gAMA (png, png_info, &_gamma_correction);

  unsigned _num_channels;
  switch (color_type)
    {
    case PNG_COLOR_TYPE_GRAY:		_num_channels = 1; break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:	_num_channels = 2; break;
    case PNG_COLOR_TYPE_RGB:		_num_channels = 3; break;
    case PNG_COLOR_TYPE_RGB_ALPHA:	_num_channels = 4; break;

    case PNG_COLOR_TYPE_PALETTE:
      png_set_palette_to_rgb (png);
      _num_channels = 3;
      break;

    default:
      png_destroy_read_struct (&png, &png_info, 0);
      fclose (stream);
      open_err ("Unsupported PNG image type");
    }

  // Expand sub-byte grey-scale bit-depths to one-byte-per-pxel
  //
  if (color_type == PNG_COLOR_TYPE_GRAY && _bit_depth < 8)
    png_set_gray_1_2_4_to_8 (png);

  // Convert a tRNS chunk to a full alpha channel.
  //
  if (png_get_valid (png, png_info, PNG_INFO_tRNS)
      && (_num_channels == 1 || _num_channels == 3))
    {
      png_set_tRNS_to_alpha (png);
      _num_channels++;
    }

  set_specs (_width, _height, _num_channels, _bit_depth);
}

PngImageSource::~PngImageSource ()
{
  if (setjmp (png_jmpbuf (png)))
    throw std::runtime_error (filename + ": Error destorying PNG object!");

  png_read_end (png, 0);
  png_destroy_read_struct (&png, &png_info, 0);

  fclose (stream);
}

void
PngImageSource::read_row (ByteVec &byte_vec)
{
  if (setjmp (png_jmpbuf (png)))
    throw std::runtime_error (filename + ": Error reading PNG file");

  png_read_row (png, static_cast<png_byte *>(&byte_vec[0]), 0);
}


// arch-tag: 034a86a6-eef0-47ad-9a04-17074e04e62e
