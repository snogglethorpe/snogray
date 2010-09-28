// image-tga.cc -- TGA ("Targa") format image handling
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "image-tga.h"

using namespace snogray;


// Input

TgaImageSource::TgaImageSource (const std::string &_filename,
				const ValTable &params)
  : ByteVecImageSource (_filename, params),
    inf (_filename, std::ios::in | std::ios::binary)
{
  unsigned char header[HEADER_LENGTH];

  read_buf (header, HEADER_LENGTH);

  unsigned encoding_type = header[HDR_ENCODING_OFFS];
  if (encoding_type != ENCODING_TRUECOLOR_UNCOMPRESSED
      && encoding_type != ENCODING_GREY_UNCOMPRESSED
      && encoding_type != ENCODING_TRUECOLOR_RLE
      && encoding_type != ENCODING_GREY_RLE)
    open_err ("Unsupported TGA image encoding");

  use_rle = (encoding_type == ENCODING_TRUECOLOR_RLE
	     || encoding_type == ENCODING_GREY_RLE);

  unsigned char color_map_type = header[HDR_COLOR_MAP_TYPE_OFFS];
  if (color_map_type != COLOR_MAP_NONE
      && color_map_type != COLOR_MAP_PRESENT)
    open_err ("Unsupported TGA color-map type");

  unsigned width = read16 (header + HDR_WIDTH_OFFS);
  unsigned height = read16 (header + HDR_HEIGHT_OFFS);

  unsigned descriptor = header[HDR_DESCRIPTOR_OFFS];
  unsigned attribute_bits = descriptor & 0xF; // "attribute" == alpha

  pixel_depth = header[HDR_PIXEL_DEPTH_OFFS];
  if (pixel_depth > 32 || (pixel_depth & 0x7) != 0)
    open_err ("Invalid TGA pixel-depth");

  if ((pixel_depth == 32 && attribute_bits != 8 && attribute_bits != 0)
      || (pixel_depth == 24 && attribute_bits != 0)
      || (pixel_depth == 16 && attribute_bits > 1)
      || (pixel_depth == 8 && attribute_bits != 0))
    open_err ("TGA pixel-depth inconsistent with attribute bits");

  PixelFormat pixel_format
    = pixel_depth == 8 ? PIXEL_FORMAT_GREY : PIXEL_FORMAT_RGB;
  if (pixel_depth == 32 || attribute_bits != 0)
    pixel_format = pixel_format_add_alpha_channel (pixel_format);

  // The 16-bit-per-pixel format uses 5-bit fields for RGB, and we
  // convert the alpha bit to 5 bits as well so that it can be handled
  // consistently by the ByteVecImageSource class.  All other formats
  // just use 8 bits per component.
  //
  unsigned bits_per_component = (pixel_depth == 16) ? 5 : 8;

  set_specs (width, height, pixel_format, 1, bits_per_component);

  // Skip to the image data
  //
  unsigned skip_bytes = header[HDR_IMAGE_ID_LEN_OFFS];
  if (color_map_type == COLOR_MAP_PRESENT)
    skip_bytes += read16 (header + HDR_COLOR_MAP_LENGTH_OFFS);
  inf.seekg (skip_bytes, std::ios::cur);

  // Allocate temporary row-buffer of the appropriate size.
  //
  row_buf.assign (width * (pixel_depth >> 3), 0); // bytes per row
}

void
TgaImageSource::read_row (ByteVec &byte_vec)
{
  unsigned bytes_per_pixel = pixel_depth >> 3;
  unsigned byte_vec_offs = 0;

  if (use_rle)
    {
      // Simple run-length-encoding.  Basically divided into spans of
      // pixels preceded by a length byte.  The low 7 bits of the
      // length byte are the number of pixels, NUM, in the span; if
      // the high bit is 0, then then length byte is followed by NUM
      // normal pixels.  If the hight bit is 1, the length byte is
      // followed by a single pixel which should be repeated NUM
      // times.

      unsigned num_pixels = 0;
      while (num_pixels < width)
	{
	  unsigned char span_len = read_byte ();
	  bool dup = (span_len & 0x80) != 0;
	  span_len &= 0x7F;	// clear dup bit
	  span_len++;		// number of pixels is encoded value + 1

	  if (num_pixels + span_len > width)
	    err ("TGA image RLE span crosses row boundary");
	    
	  unsigned pixels_to_read = dup ? 1 : span_len;
	  read_buf (&row_buf[0], pixels_to_read * bytes_per_pixel);

	  unsigned buf_offs = 0;
	  for (unsigned i = 0; i < span_len; i++)
	    {
	      decode_pixel (&row_buf[buf_offs], byte_vec, byte_vec_offs);
	      if (! dup)
		buf_offs += bytes_per_pixel;
	    }

	  num_pixels += span_len;
	}
    }
  else
    {
      // Uncompressed image data, just pixels stored in order.

      read_buf (&row_buf[0], bytes_per_pixel * width);

      unsigned buf_offs = 0;
      for (unsigned i = 0; i < width ; i++)
	{
	  decode_pixel (&row_buf[buf_offs], byte_vec, byte_vec_offs);
	  buf_offs += bytes_per_pixel;
	}
    }
}
