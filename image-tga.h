// image-tga.h -- TGA ("Targa") format image handling
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

#ifndef __IMAGE_TGA_H__
#define __IMAGE_TGA_H__

#include <fstream>
#include <vector>

#include "image-byte-vec.h"


namespace snogray {


class TgaImageSource : public ByteVecImageSource
{  
public:

  TgaImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE);

  virtual RowOrder row_order () const { return FIRST_ROW_AT_BOTTOM; }

  virtual void read_row (ByteVec &rgb_bytes);

private:

  // Color-map types; we only include those we support.
  //
  enum ColorMapType {
    COLOR_MAP_NONE    = 0,
    COLOR_MAP_PRESENT = 1
  };

  // Image encoding types; we only include those we support.
  //
  enum EncodingType {
    ENCODING_TRUECOLOR_UNCOMPRESSED = 2,
    ENCODING_GREY_UNCOMPRESSED	    = 3,
    ENCODING_TRUECOLOR_RLE	    = 10,
    ENCODING_GREY_RLE		    = 11
  };

  // Length of file header in bytes.
  //
  static const unsigned HEADER_LENGTH = 18;

  // Header field offsets.
  //
  static const unsigned HDR_IMAGE_ID_LEN_OFFS	  = 0;	// 1 byte
  static const unsigned HDR_COLOR_MAP_TYPE_OFFS	  = 1;	// 1 byte
  static const unsigned HDR_ENCODING_OFFS	  = 2;	// 1 byte
  // "Color map specification" (offset 3) fields:
  static const unsigned HDR_COLOR_MAP_LENGTH_OFFS = 3+2; // 2 bytes
  // "Image specification" (offset 8) fields:
  static const unsigned HDR_X_ORIGIN_OFFS	  = 8+0; // 2 bytes
  static const unsigned HDR_Y_ORIGIN_OFFS	  = 8+2; // 2 bytes
  static const unsigned HDR_WIDTH_OFFS		  = 8+4; // 2 bytes
  static const unsigned HDR_HEIGHT_OFFS		  = 8+6; // 2 bytes
  static const unsigned HDR_PIXEL_DEPTH_OFFS	  = 8+8; // 1 byte
  static const unsigned HDR_DESCRIPTOR_OFFS	  = 8+1; // 1 byte

  // Return the value of a 2-byte number at memory MEM, encoded in the TGA
  // standard encodig (little-endian).
  //
  static unsigned read16 (const unsigned char *mem)
  {
    return mem[0] + (mem[1] << 8);
  }

  // Read a 16-bit pixel from memory at FROM, and write it into
  // BYTE_VEC starting at offset BYTE_VEC_OFFS.  BYTE_VEC_OFFS is
  // incremented by the number of bytes stored into BYTE_VEC.
  //
  void decode_16_bit_pixel (const unsigned char *from,
			    ByteVec &byte_vec, unsigned &byte_vec_offs)
  {
    unsigned pixel = read16 (from);
    byte_vec[byte_vec_offs++] = pixel & 0x1F;
    byte_vec[byte_vec_offs++] = (pixel >> 5) & 0x1F;
    byte_vec[byte_vec_offs++] = (pixel >> 10) & 0x1F;
    if (pixel_format == PIXEL_FORMAT_RGBA)
      byte_vec[byte_vec_offs++] = (pixel >> 15) & 1;
  }

  // Read a pixel from memory at FROM, and write it into BYTE_VEC
  // starting at offset BYTE_VEC_OFFS.  BYTE_VEC_OFFS is incremented
  // by the number of bytes stored into BYTE_VEC.
  //
  void decode_pixel (const unsigned char *from,
		     ByteVec &byte_vec, unsigned &byte_vec_offs)
  {
    if (pixel_depth == 16)
      decode_16_bit_pixel (from, byte_vec, byte_vec_offs);
    else if (pixel_depth == 8)
      byte_vec[byte_vec_offs++] = from[0];
    else
      {
	byte_vec[byte_vec_offs++] = from[0];
	byte_vec[byte_vec_offs++] = from[1];
	byte_vec[byte_vec_offs++] = from[2];
	if (pixel_format == PIXEL_FORMAT_RGBA)
	  byte_vec[byte_vec_offs++] = from[3]; // alpha/attribute byte
      }
  }

  // Read a block of length LEN from our input file into BUF.  This is
  // mainly a convenience function because the standard iostream read
  // method reads into a char* buffer, whereas we want unsigned char*.
  //
  void read_buf (unsigned char *buf, size_t len)
  {
    inf.read (reinterpret_cast<char *> (buf), len);
  }

  // Read a single byte from our input file and return it as an
  // unsigned number.
  //
  unsigned read_byte ()
  {
    return static_cast<unsigned> (inf.get ());
  }

  // Number of bits in a pixel (8, 16, 24, or 32).
  //
  unsigned pixel_depth;

  // Temporary buffer for use when reading rows.
  //
  std::vector<unsigned char> row_buf;

  // True if this file uses run-length-encoding (otherwise it's
  // uncompressed).
  //
  bool use_rle;

  // Input file.
  //
  std::ifstream inf;
};


}

#endif // __IMAGE_TGA_H__