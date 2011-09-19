// image-tga.h -- TGA ("Targa") format image handling
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_TGA_H
#define SNOGRAY_IMAGE_TGA_H

#include <fstream>
#include <vector>

#include "image-byte-vec.h"


namespace snogray {


// Common definitions for TGA input and output.
//
struct TgaDefs
{
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
  static const unsigned HDR_DESCRIPTOR_OFFS	  = 8+9; // 1 byte
};


// Output

class TgaImageSink : public ByteVecImageSink, TgaDefs
{  
public:

  TgaImageSink (const std::string &filename,
		unsigned width, unsigned height,
		const ValTable &params = ValTable::NONE);
  ~TgaImageSink ();

  virtual void write_row (const ByteVec &byte_vec);

  // Write previously written rows to disk, if possible.  This may flush
  // I/O buffers etc., but will not in any way change the output (so for
  // instance, it will _not_ flush the compression state of a TGA output
  // image, as that can make the resulting compression worse).
  //
  virtual void flush () { outf.flush (); }

private:

  // Write out BYTE_VEC using TGA's simple run-length-encoding format.
  //
  // Each row is divided into spans of pixels preceded by a length
  // byte.  The low 7 bits of the length byte are the number of pixels
  // in the following span, NUM, minus 1.  If the high bit is 0, then
  // then length byte is followed by NUM normal pixels.  If the hight
  // bit is 1, the length byte is followed by a single pixel which
  // should be repeated NUM times.
  //
  void write_rle_row (const ByteVec &byte_vec);

  // Write a span of pixels in BYTE_VEC, from from START_OFFS to
  // END_OFFS, in the TGA RLE format using repeat-mode REPEAT.
  //
  void write_rle_span (const ByteVec &byte_vec,
		       unsigned start_offs, unsigned end_offs,
		       bool repeat);

  // Write a single pixel from offset OFFS in the RGB[A] byte-vector
  // BYTE_VEC to the output stream.
  //
  void write_pixel (const ByteVec &byte_vec, unsigned offs)
  {
    // Color components in BYTE_VEC are stored in RGB order.
    unsigned r = byte_vec[offs];
    unsigned g = byte_vec[offs + 1];
    unsigned b = byte_vec[offs + 2];

    // ... and we write them to the image file in TGA BGR order.
    //
    outf.put (b);
    outf.put (g);
    outf.put (r);

    // If there's an alpha channel, write that too.
    //
    if (alpha_bytes)
      outf.put (byte_vec[offs + 3]);
  }

  // Store VAL as a 2-byte number at memory location MEM, encoded in
  // the TGA standard encoding (little-endian).
  //
  static void put16 (unsigned char *mem, unsigned val)
  {
    mem[0] = val & 0xFF;
    mem[1] = (val >> 8) & 0xFF;
  }

  // Output stream.
  //
  std::ofstream outf;

  // 1 if we have an alpha channel, 0 if not (used both as a boolean,
  // and in some arithmetic).
  //
  unsigned alpha_bytes;

  // Size in bytes of a pixel in BYTE_VEC.
  //
  unsigned pixel_bytes;
};


// Input

class TgaImageSource : public ByteVecImageSource, TgaDefs
{  
public:

  TgaImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE);

  virtual RowOrder row_order () const { return _row_order; }

  virtual void read_row (ByteVec &rgb_bytes);

private:

  // Return the value of a 2-byte number at memory MEM, encoded in the TGA
  // standard encodig (little-endian).
  //
  static unsigned read16 (const unsigned char *mem)
  {
    return mem[0] + (mem[1] << 8);
  }

  // Read a pixel from memory at FROM, and write it into BYTE_VEC
  // starting at offset BYTE_VEC_OFFS.  BYTE_VEC_OFFS is incremented
  // by the number of bytes stored into BYTE_VEC.
  //
  void decode_pixel (const unsigned char *from,
		     ByteVec &byte_vec, unsigned &byte_vec_offs)
  {
    if (bytes_per_pixel == 1)
      // Single byte of grey-level
      {
	byte_vec[byte_vec_offs++] = from[0];
      }
    else if (bytes_per_pixel == 2)
      // Three five-bit RGB components and a single alpha/"attribute"
      // bit packed into a 16-bit word:  (MSB) ARRRRRGGGGGBBBBB (LSB)
      //
      // Note that in this case we've already told our superclass to
      // expect 5-bit fields (which it scales properly to cover a
      // floating-point range of 0-1); we convert the single-bit
      // alpha/attribute field to a value of either 0 or 31 so that it
      // covers the same range as the other components.
      {
	unsigned pixel = read16 (from);
	byte_vec[byte_vec_offs++] = (pixel >> 10) & 0x1F; // red
	byte_vec[byte_vec_offs++] = (pixel >> 5) & 0x1F;  // blue
	byte_vec[byte_vec_offs++] = (pixel & 0x1F);	  // green
	if (pixel_format == PIXEL_FORMAT_RGBA)
	  // convert single high alpha bit to either 0 or 31
	  byte_vec[byte_vec_offs++] = ((pixel >> 10) & 0x20) - 1;
      }
    else
      // Three single-byte RGB components, and optionally a single
      // alpha byte.  Note that the component bytes are ordered B-G-R
      // in the TGA file, which we reverse to R-G-B order when storing
      // into BYTE_VEC.
      {
	byte_vec[byte_vec_offs++] = from[2]; // red is last color in TGA
	byte_vec[byte_vec_offs++] = from[1]; // green is middle
	byte_vec[byte_vec_offs++] = from[0]; // blue is first
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

  // Number of bytes in a pixel (1-4).
  //
  unsigned bytes_per_pixel;

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

  // Order of rows in the file.
  //
  RowOrder _row_order;
};


}

#endif // SNOGRAY_IMAGE_TGA_H
