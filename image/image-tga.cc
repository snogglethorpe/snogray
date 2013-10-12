// image-tga.cc -- TGA ("Targa") format image handling
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstring> // memset

#include "image-tga.h"


using namespace snogray;



// ----------------------------------------------------------------
// TgaImageSink: TGA image output


TgaImageSink::TgaImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const ValTable &params)
  : ByteVecImageSink (filename, width, height, params),
    outf (filename.c_str(),
	  std::ios_base::out|std::ios_base::binary|std::ios_base::trunc),
    alpha_bytes (has_alpha_channel () ? 1 : 0),
    pixel_bytes (3 + alpha_bytes)
{
  // Header block at beginning of file.
  //
  unsigned char header[HEADER_LENGTH];

  memset (header, 0, HEADER_LENGTH); // just in case

  //
  // For now, we just use a fixed set of parameters: true-color,
  // run-length-encoded (RLE), no color-map, first-row-at-top.
  //

  // image encoding & color-map
  header[HDR_ENCODING_OFFS] = ENCODING_TRUECOLOR_RLE;
  header[HDR_COLOR_MAP_TYPE_OFFS] = COLOR_MAP_NONE;

  // image size
  put16 (header + HDR_WIDTH_OFFS, width);
  put16 (header + HDR_HEIGHT_OFFS, height);

  // image attributes
  unsigned descriptor = 0;
  descriptor |= 0x20;			     // FIRST_ROW_AT_TOP
  descriptor |= has_alpha_channel() ? 8 : 0; // number of "attribute" bits
  header[HDR_DESCRIPTOR_OFFS] = descriptor;

  // pixel size; we fix it at 24 or 32 bits, without or with an alpha channel
  header[HDR_PIXEL_DEPTH_OFFS] = has_alpha_channel() ? 32 : 24;

  // other misc fields
  header[HDR_IMAGE_ID_LEN_OFFS] = 0;
  header[HDR_X_ORIGIN_OFFS] = 0;
  header[HDR_Y_ORIGIN_OFFS] = 0;

  // Write the header block.
  //
  outf.write (reinterpret_cast<char *> (header), HEADER_LENGTH);
}


TgaImageSink::~TgaImageSink () {}


void
TgaImageSink::write_row (const ByteVec &byte_vec)
{
  write_rle_row (byte_vec);
}


// Write out BYTE_VEC using TGA's simple run-length-encoding format.
//
// Each row is divided into spans of pixels preceded by a length byte.
// The low 7 bits of the length byte are the number of pixels in the
// following span, NUM, minus 1.  If the high bit is 0, then then
// length byte is followed by NUM normal pixels.  If the hight bit is
// 1, the length byte is followed by a single pixel which should be
// repeated NUM times.
//
void
TgaImageSink::write_rle_row (const ByteVec &byte_vec)
{
  // Offset in BYTE_VEC of the start of the current span, in bytes.
  //
  unsigned span_offs = 0;

  // Size of the current span, so far, in pixels.
  //
  unsigned span_size = 0;

  // Whether the current span we're accumulating is a "repeat span"
  // (where all pixels have the same value).  When SPAN_SIZE < 2,
  // this has no meaning (as the kind of span isn't yet decided).
  //
  bool is_repeat_span = false;

  // Pixel component values of the _previous_ pixel examined.
  //
  unsigned prev_r = 0, prev_g = 0, prev_b = 0, prev_a = 0;

  for (unsigned offs = 0; offs < byte_vec.size (); offs += pixel_bytes)
    {
      unsigned r = byte_vec[offs];
      unsigned g = byte_vec[offs + 1];
      unsigned b = byte_vec[offs + 2];
      unsigned a = alpha_bytes ? byte_vec[offs + 3] : 0;

      // Unless we're at the start of a span, see if this is a repeat
      // pixel.
      //
      if (span_size != 0)
	{
	  // See if this pixel is the same as the previous pixel.
	  //
	  bool is_repeat_pixel = (r == prev_r && g == prev_g && b == prev_b);

	  // If we have an alpha-channel, check the alpha-component too.
	  //
	  if (alpha_bytes && is_repeat_pixel)
	    is_repeat_pixel = (a == prev_a);

	  if (span_size == 1)
	    {
	      // The span wasn't long enough to be marked as repeating
	      // or non-repeating, so set it now.

	      is_repeat_span = is_repeat_pixel;
	    }
	  else if (is_repeat_pixel != is_repeat_span)
	    {
	      // The "repeatedness" of the current/prev pixel doesn't
	      // match the type of span we're currently accumulating,
	      // write out the current span and start a new one.

	      // Where the next span will start.
	      //
	      unsigned new_span_offs
		= is_repeat_pixel ? offs - pixel_bytes : offs;

	      // Write the span.
	      //
	      write_rle_span (byte_vec, span_offs, new_span_offs,
			      is_repeat_span);

	      // Start the new span, which includes the previous pixel
	      // if we're starting a repeat span.
	      //
	      span_size = is_repeat_pixel ? 1 : 0;
	      span_offs = new_span_offs;
	      is_repeat_span = is_repeat_pixel;
	    }
	  else if (span_size == 128)
	    {
	      // The current span's repeat count would overflow a
	      // 7-bit counter, write it out anyway; the maximum
	      // repeat count is 128, not 127, because the actual
	      // value stored is COUNT - 1.

	      write_rle_span (byte_vec, span_offs, offs, is_repeat_span);
	      span_size = 0;
	      span_offs = offs;
	      // is_repeat_span doesn't change.
	    }
	}

      // Include the current pixel in the current span.
      //
      span_size++;

      // Remember this pixel so we can see whether the next pixel
      // matches it.
      //
      prev_r = r;
      prev_g = g;
      prev_b = b;
      prev_a = a;
    }

  // Finally, write any span currently being accumulated.
  //
  if (span_size != 0)
    write_rle_span (byte_vec, span_offs, byte_vec.size(), is_repeat_span);
}

// Write a span of pixels in BYTE_VEC, from from START_OFFS to
// END_OFFS, in the TGA RLE format using repeat-mode REPEAT.
//
void
TgaImageSink::write_rle_span (const ByteVec &byte_vec,
			      unsigned start_offs, unsigned end_offs,
			      bool repeat)
{
  // The number of pixels to be written, minus one (this is the count
  // used in the TGA file).
  //
  unsigned pixel_count = (end_offs - start_offs) / pixel_bytes - 1;

  if (repeat)
    {
      // "repeat" span, write out the repeat-count, and the single value.
      outf.put (0x80 | pixel_count);
      write_pixel (byte_vec, start_offs);
    }
  else
    {
      // "non-repeat" span, write out the pixel count, and the pixel values.
      outf.put (pixel_count);
      for (unsigned offs = start_offs; offs < end_offs; offs += pixel_bytes)
	write_pixel (byte_vec, offs);
    }
}



// ----------------------------------------------------------------
// TgaImageSource: TGA image input


TgaImageSource::TgaImageSource (const std::string &_filename,
				const ValTable &params)
  : ByteVecImageSource (_filename, params),
    inf (_filename.c_str(), std::ios::in | std::ios::binary),
    _row_order (FIRST_ROW_AT_BOTTOM) // default
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

  unsigned width = get16 (header + HDR_WIDTH_OFFS);
  unsigned height = get16 (header + HDR_HEIGHT_OFFS);

  unsigned descriptor = header[HDR_DESCRIPTOR_OFFS];
  unsigned attribute_bits = descriptor & 0xF; // "attribute" == alpha

  // Descriptor bit 0x20 is the "y origin at top of image" flag.
  //
  _row_order = (descriptor & 0x20) ? FIRST_ROW_AT_TOP : FIRST_ROW_AT_BOTTOM;

  unsigned pixel_depth = header[HDR_PIXEL_DEPTH_OFFS];
  if (pixel_depth > 32 || (pixel_depth & 0x7) != 0)
    open_err ("Invalid TGA pixel-depth");

  bytes_per_pixel = pixel_depth >> 3;

  // Verify that the number of "attribute" (usually alpha) bits is
  // valid for the pixel format.  Only RGB images actually have
  // alpha/attributes stored in the image data, but greyscale images
  // can sometimes have a non-zero attribute_bits field, so we just
  // ignore it in that case.
  //
  if ((bytes_per_pixel == 4 && attribute_bits != 8 && attribute_bits != 0)
      || (bytes_per_pixel == 3 && attribute_bits != 0)
      || (bytes_per_pixel == 2 && attribute_bits > 1))
    open_err ("TGA pixel-depth inconsistent with attribute bits");

  // Set the final pixel format.  Note that only RGB images can have
  // an alpha channel in the TGA format.
  //
  PixelFormat pixel_format
    = (bytes_per_pixel == 1) ? PIXEL_FORMAT_GREY : PIXEL_FORMAT_RGB;
  if (pixel_format == PIXEL_FORMAT_RGB
      && (bytes_per_pixel == 4 || attribute_bits != 0))
    pixel_format = pixel_format_add_alpha_channel (pixel_format);

  // The 16-bit-per-pixel format uses 5-bit fields for RGB, and we
  // convert the alpha bit to 5 bits as well so that it can be handled
  // consistently by the ByteVecImageSource class.  All other formats
  // just use 8 bits per component.
  //
  unsigned bits_per_component = (bytes_per_pixel == 2) ? 5 : 8;

  set_specs (width, height, pixel_format, 1, bits_per_component);

  // Skip to the image data
  //
  unsigned skip_bytes = header[HDR_IMAGE_ID_LEN_OFFS];
  if (color_map_type == COLOR_MAP_PRESENT)
    skip_bytes += get16 (header + HDR_COLOR_MAP_LENGTH_OFFS);
  inf.seekg (skip_bytes, std::ios::cur);

  // Allocate temporary row-buffer of the appropriate size.
  //
  row_buf.assign (width * bytes_per_pixel, 0); // bytes per row
}


void
TgaImageSource::read_row (ByteVec &byte_vec)
{
  unsigned byte_vec_offs = 0;

  if (use_rle)
    {
      // Simple run-length-encoding.  Basically divided into spans of
      // pixels preceded by a length byte.  The low 7 bits of the
      // length byte are the number of pixels in the following span,
      // NUM, minus 1.  If the high bit is 0, then then length byte is
      // followed by NUM normal pixels.  If the hight bit is 1, the
      // length byte is followed by a single pixel which should be
      // repeated NUM times.

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
