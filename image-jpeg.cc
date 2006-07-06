// image-jpeg.cc -- JPEG format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "image-jpeg.h"

using namespace Snogray;


// Output

JpegImageSink::JpegImageSink (const std::string &filename,
			      unsigned width, unsigned height,
			      const Params &params)
  : ByteVecImageSink (filename, width, height, params)
{
  int quality = params.get_int ("quality", DEFAULT_QUALITY);

  if (quality < 0 || quality > 100)
    open_err ("Invalid quality setting; must be in the range 0-100");

  // Open output file

  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ();

  // Create libjpeg data structures

  jpeg_info.err = jpeg_std_error (&jpeg_err);

  jpeg_create_compress (&jpeg_info);

  jpeg_stdio_dest (&jpeg_info, stream);

  // Initialize compression parameters

  jpeg_info.image_width = width;
  jpeg_info.image_height = height;
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
JpegImageSink::write_row (const ByteVec &byte_vec)
{
  const JSAMPLE *rows[1] = { &byte_vec[0] };

  jpeg_write_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
}

// Write previously written rows to disk, if possible.  This may flush
// I/O buffers etc., but will not in any way change the output (so for
// instance, it will _not_ flush the compression state of a PNG output
// image, as that can make the resulting compression worse).
//
void
JpegImageSink::flush ()
{
  fflush (stream);
}


// Input

JpegImageSource::JpegImageSource (const std::string &filename,
				  const Params &params)
  : ByteVecImageSource (filename, params)
{
  // Open input file

  stream = fopen (filename.c_str(), "rb");
  if (! stream)
    open_err ();

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
JpegImageSource::read_row (ByteVec &byte_vec)
{
  const JSAMPLE *rows[1] = { &byte_vec[0] };
  jpeg_read_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
}


// arch-tag: 4abc6a5f-8aeb-4b58-a253-36553d2b109f
