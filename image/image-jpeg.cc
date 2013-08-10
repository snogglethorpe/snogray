// image-jpeg.cc -- JPEG format image handling
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "excepts.h"
#include "string-funs.h"

#include "image-jpeg.h"

using namespace snogray;


// Common code for libjpeg access

JpegErrState::JpegErrState (const std::string &_filename)
  : err (false), err_filename (_filename)
{
  jpeg_std_error (this);

  error_exit = libjpeg_err_handler;
  emit_message = libjpeg_warn_handler;
  output_message = libjpeg_msg_handler;
}

// If an error was seen, throw an appropriate exception.
//
void
JpegErrState::throw_err ()
{
  throw std::runtime_error (err_msg);
}

// Called for fatal errors.
//
void
JpegErrState::libjpeg_err_handler (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  // Get the error message.
  //
  (*cinfo->err->format_message) (cinfo, buffer);

  JpegErrState *state = static_cast<JpegErrState *> (cinfo->err);

  state->err = true;
  state->err_msg = state->err_filename + ": " + buffer;

  longjmp (state->jmpbuf, 1);
}

// Called for warnings (MSG_LEVEL < 0) and "trace messages" (>= 0).
//
void
JpegErrState::libjpeg_warn_handler (j_common_ptr cinfo, int msg_level)
{
  struct jpeg_error_mgr * err = cinfo->err;

  if (msg_level < 0)
    (*err->error_exit) (cinfo);	      // Treat "warnings" as hard errors
  else
    if (err->trace_level >= msg_level)
      (*err->output_message) (cinfo); // Just display message if enabled
}

// Call to output a message.
//
void
JpegErrState::libjpeg_msg_handler (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  // Get the message to display.
  //
  (*cinfo->err->format_message) (cinfo, buffer);

  JpegErrState *state = static_cast<JpegErrState *> (cinfo->err);

  std::cerr << state->err_filename << ": " << buffer << std::endl;
}


// Output

JpegImageSink::JpegImageSink (const std::string &filename,
			      unsigned width, unsigned height,
			      const ValTable &params)
  : ByteVecImageSink (filename, width, height, params), jpeg_err (filename)
{
  int quality = params.get_int ("quality", DEFAULT_QUALITY);

  if (quality < 0 || quality > 100)
    open_err ("Invalid quality setting; must be in the range 0-100");

  if (bytes_per_component != 1)
    open_err ("jpeg format does not support "
	      + stringify (bytes_per_component)
	      + " bytes-per-component");

  // Turn off any alpha-channel.
  //
  if (pixel_format_has_alpha_channel (pixel_format))
    pixel_format = pixel_format_base (pixel_format);

  if (pixel_format != PIXEL_FORMAT_RGB)
    open_err ("jpeg only supports the RGB pixel format");

  // Open output file

  stream = fopen (filename.c_str(), "wb");
  if (! stream)
    open_err ();

  // Create libjpeg data structures

  jpeg_info.err = &jpeg_err;

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
  //
  if (! jpeg_err.trap_err ())
    jpeg_start_compress (&jpeg_info, true);
  else
    jpeg_err.throw_err ();
}

JpegImageSink::~JpegImageSink ()
{
  if (! jpeg_err.err)
    jpeg_finish_compress (&jpeg_info);

  jpeg_destroy_compress (&jpeg_info);

  fclose (stream);
}

void
JpegImageSink::write_row (const ByteVec &byte_vec)
{
  const JSAMPLE *rows[1] = { &byte_vec[0] };

  if (! jpeg_err.trap_err ())
    jpeg_write_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
  else
    jpeg_err.throw_err ();
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
				  const ValTable &params)
  : ByteVecImageSource (filename, params), jpeg_err (filename)
{
  // Open input file

  stream = fopen (filename.c_str(), "rb");
  if (! stream)
    open_err ();

  // Create libjpeg data structures

  jpeg_info.err = &jpeg_err;

  jpeg_create_decompress (&jpeg_info);

  jpeg_stdio_src (&jpeg_info, stream);

  if (! jpeg_err.trap_err ())
    {
      // Read image header
      //
      jpeg_read_header (&jpeg_info, true);

      // Start decompressor
      //
      jpeg_start_decompress (&jpeg_info);
    }
  else
    jpeg_err.throw_err ();

  PixelFormat pxfmt;
  if (jpeg_info.output_components == 1)
    pxfmt = PIXEL_FORMAT_GREY;
  else if (jpeg_info.output_components == 3)
    pxfmt = PIXEL_FORMAT_RGB;
  else
    open_err ("unknown number of channels");

  set_specs (jpeg_info.output_width, jpeg_info.output_height, pxfmt, 1);
}

JpegImageSource::~JpegImageSource ()
{
  if (! jpeg_err.err)
    jpeg_finish_decompress (&jpeg_info);

  jpeg_destroy_decompress (&jpeg_info);

  fclose (stream);
}

void
JpegImageSource::read_row (ByteVec &byte_vec)
{
  const JSAMPLE *rows[1] = { &byte_vec[0] };

  if (! jpeg_err.trap_err ())
    jpeg_read_scanlines (&jpeg_info, const_cast<JSAMPLE **>(rows), 1);
  else
    jpeg_err.throw_err ();
}


// arch-tag: 4abc6a5f-8aeb-4b58-a253-36553d2b109f
