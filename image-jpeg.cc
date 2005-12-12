// image-jpeg.cc -- JPEG format image handling
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

#include <jpeglib.h>

#include "image-jpeg.h"

using namespace Snogray;


// Output

class JpegImageSink : public ByteVecImageSink
{  
public:
  JpegImageSink (const JpegImageSinkParams &params);
  ~JpegImageSink ();

  virtual void write_row (const byte *rgb_bytes);

private:
  FILE *stream;

  struct jpeg_compress_struct jpeg_info;
  struct jpeg_error_mgr jpeg_err;
};

JpegImageSink::JpegImageSink (const JpegImageSinkParams &params)
  : ByteVecImageSink (params)
{
  int quality = (int)params.quality;

  if (! quality)
    quality = (int)ImageSinkParams::DEFAULT_QUALITY;
  else if (quality < 0 || quality > 100)
    params.error ("Invalid quality setting; must be in the range 0-100");

  // Open output file

  stream = fopen (params.file_name, "wb");
  if (! stream)
    params.sys_error ("Could not open output file");

  // Create libjpeg data structures

  jpeg_info.err = jpeg_std_error (&jpeg_err);

  jpeg_create_compress (&jpeg_info);

  jpeg_stdio_dest (&jpeg_info, stream);

  // Initialize compression parameters

  jpeg_info.image_width = params.width;
  jpeg_info.image_height = params.height;
  jpeg_info.input_components = 3;	/* # of color components per pixel */
  jpeg_info.in_color_space = JCS_RGB; 	/* colorspace of input image */

  jpeg_set_defaults (&jpeg_info);

  jpeg_set_quality (&jpeg_info, quality, true);

  // Start compressor

  jpeg_start_compress (&jpeg_info, true);
}

JpegImageSink::~JpegImageSink ()
{
  jpeg_finish_compress (&jpeg_info);
  jpeg_destroy_compress (&jpeg_info);
  fclose (stream);
}

void
JpegImageSink::write_row (const byte *byte_vec)
{
  const JSAMPLE *rows[1] = { byte_vec };
  jpeg_write_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
}

ImageSink *
JpegImageSinkParams::make_sink () const
{
  return new JpegImageSink (*this);
}


// Input

class JpegImageSource : public ByteVecImageSource
{  
public:
  JpegImageSource (const JpegImageSourceParams &params);
  ~JpegImageSource ();

  virtual void read_row (byte *rgb_bytes);

private:
  FILE *stream;

  struct jpeg_decompress_struct jpeg_info;
  struct jpeg_error_mgr jpeg_err;
};

JpegImageSource::JpegImageSource (const JpegImageSourceParams &params)
  : ByteVecImageSource (params)
{
  // Open input file

  stream = fopen (params.file_name, "rb");
  if (! stream)
    params.sys_error ("Could not open input file");

  // Create libjpeg data structures

  jpeg_info.err = jpeg_std_error (&jpeg_err);

  jpeg_create_decompress (&jpeg_info);

  jpeg_stdio_src (&jpeg_info, stream);

  // Read image header

  jpeg_read_header (&jpeg_info, true);

  // Start decompressor

  jpeg_start_decompress (&jpeg_info);

  set_specs (jpeg_info.output_width, jpeg_info.output_height,
	     jpeg_info.output_components, 8);
}

JpegImageSource::~JpegImageSource ()
{
  jpeg_finish_decompress (&jpeg_info);
  jpeg_destroy_decompress (&jpeg_info);
  fclose (stream);
}

void
JpegImageSource::read_row (byte *byte_vec)
{
  const JSAMPLE *rows[1] = { byte_vec };
  jpeg_read_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
}

ImageSource *
JpegImageSourceParams::make_source () const
{
  return new JpegImageSource (*this);
}


// arch-tag: 4abc6a5f-8aeb-4b58-a253-36553d2b109f
