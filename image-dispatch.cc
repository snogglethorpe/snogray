// image-dispatch.cc -- Image backend selection
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstring>

#include "image-io.h"

#include "image-exr.h"
#include "image-png.h"
#include "image-jpeg.h"
#include "image-ppm.h"
#include "image-pfm.h"

using namespace Snogray;

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
  else if (strcasecmp (fmt, "ppm") == 0)
    return PpmImageSinkParams (*this).make_sink ();
  else if (strcasecmp (fmt, "pfm") == 0)
    return PfmImageSinkParams (*this).make_sink ();
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
  else if (strcasecmp (fmt, "png") == 0)
    return PngImageSourceParams (*this).make_source ();
  else if (strcasecmp (fmt, "jpeg") == 0 || strcasecmp (fmt, "jpg") == 0)
    return JpegImageSourceParams (*this).make_source ();
  else if (strcasecmp (fmt, "ppm") == 0)
    return PpmImageSourceParams (*this).make_source ();
  else if (strcasecmp (fmt, "pfm") == 0)
    return PfmImageSourceParams (*this).make_source ();
  else
    error ("Unknown or unsupported input image type");
  return 0; // gcc fails to notice ((noreturn)) attribute on `error' method
}

// Returns true if FILENAME is a recogized image format we can read.
//
bool
ImageInput::recognized_filename (const std::string &filename)
{
  unsigned dot = filename.find_last_of (".");
  if (dot == filename.length ())
    return false;

  std::string ext = filename.substr (dot + 1);

  return ext == "exr" || ext == "png" || ext == "jpeg" || ext == "jpg"
    || ext == "ppm" || ext == "pfm";
}

// arch-tag: df36e3bf-7e23-4f22-91a3-03a954777784
