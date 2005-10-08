// image.cc -- Image datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <string>
#include <cstring>
#include <cerrno>

#include "image.h"

// For image-type dispatch
#include "image-exr.h"
#include "image-png.h"
#include "image-jpeg.h"

using namespace Snogray;


const float ImageSinkParams::DEFAULT_TARGET_GAMMA;
const float ImageSinkParams::DEFAULT_QUALITY;

const ImageOutput::aa_filter_t DEFAULT_AA_FILTER = ImageOutput::aa_gauss_filter;



// Calls error with current errno message appended
void
ImageParams::sys_error (const std::string &msg) const
{
  std::string buf (msg);
  buf += ": ";
  buf += strerror (errno);
  error (buf);
}

// Return the file format to use; if the FORMAT field is 0, then try
// to guess it from FILE_NAME.
const char *
ImageParams::find_format () const
{
  if (format)
    // Format is user-specified
    return format;

  // Otherwise guess the output format automatically we can

  if (! file_name)
    error ("Image file type must be specified for stream I/O");
      
  const char *file_ext = rindex (file_name, '.');

  if (! file_ext)
    error ("No filename extension to determine image type");

  return file_ext + 1;
}

ImageSink *
ImageSinkParams::make_sink () const
{
  const char *fmt = find_format ();

  // Make the output-format-specific parameter block
  if (strcasecmp (fmt, "exr") == 0)
    return ExrImageSinkParams (*this).make_sink ();
  else if (strcasecmp (fmt, "png") == 0)
    return PngImageSinkParams (*this).make_sink ();
  else if (strcasecmp (fmt, "jpeg") == 0 || strcasecmp (fmt, "jpg") == 0)
    return JpegImageSinkParams (*this).make_sink ();
  else
    error ("Unknown or unsupported output image type");
  return 0; // gcc fails to notice ((noreturn)) attribute on `error' method
}

ImageSource *
ImageSourceParams::make_source () const
{
  const char *fmt = find_format ();

  // Make the output-format-specific parameter block
  if (strcasecmp (fmt, "exr") == 0)
    return ExrImageSourceParams (*this).make_source ();
#if 0
  else if (strcasecmp (fmt, "png") == 0)
    return PngImageSourceParams (*this).make_source ();
  else if (strcasecmp (fmt, "jpeg") == 0 || strcasecmp (fmt, "jpg") == 0)
    return JpegImageSourceParams (*this).make_source ();
#endif
  else
    error ("Unknown or unsupported input image type");
  return 0; // gcc fails to notice ((noreturn)) attribute on `error' method
}


// ImageOutput constructor/destructor

ImageOutput::ImageOutput (const ImageSinkParams &params)
  : aa_factor (params.aa_factor), sink (params.make_sink ())
{
  aa_filter_t aa_filter = params.aa_filter;

  // Assign defaults
  if (aa_factor == 0)
    aa_factor = 1;
  if (! aa_filter)
    aa_filter = ImageOutput::aa_gauss_filter;

  aa_kernel_size = aa_factor + params.aa_overlap*2;

  if (aa_kernel_size > 1)
    {
      aa_row = new ImageRow (params.width);
      aa_kernel = make_aa_kernel (aa_filter, aa_kernel_size);
      aa_max_intens = sink->max_intens ();
    }
  else
    {
      aa_row = 0;
      aa_kernel = 0;
    }

  recent_rows = new ImageRow*[aa_kernel_size];
  for (unsigned offs = 0; offs < aa_kernel_size; offs++)
    recent_rows[offs] = new ImageRow (params.width * aa_factor);

  next_row_offs = 0;
  num_accumulated_rows = 0;
}

ImageSink::~ImageSink () { }

ImageOutput::~ImageOutput ()
{
  write_accumulated_rows ();

  delete sink;

  for (unsigned offs = 0; offs < aa_factor; offs++)
    delete recent_rows[offs];
  delete[] recent_rows;

  if (aa_row)
    delete aa_row;
  if (aa_kernel)
    delete[] aa_kernel;
}

void
ImageOutput::write_accumulated_rows ()
{
  if (num_accumulated_rows)
    {
      if (aa_kernel_size > 1)
	// Anti-alias recent rows and write the result to our output file
	{
	  fill_aa_row ();
	  sink->write_row (*aa_row);
	}
      else
	// For non-anti-aliased output, just write what we have
	sink->write_row (*recent_rows[0]);

      num_accumulated_rows = 0;
    }
}

ImageRow &
ImageOutput::next_row ()
{
  if (num_accumulated_rows >= aa_factor)
    // Once we accumulate enough output rows, write to our output sink
    write_accumulated_rows ();

  num_accumulated_rows++;

  if (next_row_offs >= aa_kernel_size)
    next_row_offs = 0;

  return *recent_rows[next_row_offs++];
}


// Stubs

ImageParams::~ImageParams () { }

ImageSource::~ImageSource () { }

float
ImageSink::max_intens () const
{
  return 0;			// no (meaningful) maximum, i.e. floating-point
}

void
ImageFmtSinkParams::error (const std::string &msg) const
{
  generic_params->error (msg);
}

void
ImageFmtSourceParams::error (const std::string &msg) const
{
  generic_params->error (msg);
}

// arch-tag: 3e9296c6-5ac7-4c39-8b79-45ce81b5d480
