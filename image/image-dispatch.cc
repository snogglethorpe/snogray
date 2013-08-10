// image-dispatch.cc -- Image backend selection
//
//  Copyright (C) 2005, 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include "excepts.h"
#include "string-funs.h"

#include "image-pfm.h"
#include "image-rgbe.h"
#include "image-tga.h"
#ifdef HAVE_LIBEXR
# include "image-exr.h"
#endif
#ifdef HAVE_LIBPNG
# include "image-png.h"
#endif
#ifdef HAVE_LIBJPEG
# include "image-jpeg.h"
#endif
#ifdef HAVE_LIBNETPBM
# include "image-ppm.h"
#endif

#include "image-io.h"


using namespace snogray;


// Filename format deduction

// If PARAMS contains an explicit "format" entry, return its value,
// otherwise if FILENAME has a recognized extension from which we can
// guess its format, return it (converted to lower-case).
//
std::string
ImageIo::find_format (const ValTable &params, const std::string &filename)
{
  std::string fmt = params.get_string ("format");

  if (fmt.empty ())
    // No explicitly specified format, try looking at the file name
    //
    fmt = filename_ext (filename);

  if (fmt.empty ())
    throw std::runtime_error ("Cannot determine file type");

  return fmt;
}

// Return true if FILENAME has a recogized image format we can read.
//
bool
ImageIo::recognized_filename (const std::string &filename)
{
  std::string fmt = filename_ext (filename);

  return
    fmt == "pfm" || fmt == "rgbe" || fmt == "hdr" || fmt == "pic"
    || fmt == "tga"
#ifdef HAVE_LIBEXR
    || fmt == "exr"
#endif
#ifdef HAVE_LIBPNG
    || fmt == "png"
#endif
#ifdef HAVE_LIBJPEG
    || fmt == "jpeg" || fmt == "jpg"
#endif
#ifdef HAVE_LIBNETPBM
    || fmt == "ppm"
#endif
    ;
}



ImageSink *
ImageSink::open (const std::string &filename, unsigned width, unsigned height,
		 const ValTable &params)
{
  std::string fmt = find_format (params, filename);

  // Formats we always support
  //
  if (fmt == "pfm")
    return new PfmImageSink (filename, width, height, params);
  else if (fmt == "rgbe" || fmt == "hdr" || fmt == "pic")
    return new RgbeImageSink (filename, width, height, params);
  else if (fmt == "tga" || fmt == "targa")
    return new TgaImageSink (filename, width, height, params);

  // Formats which are only supported if an appropriate library  is available
  //
#ifdef HAVE_LIBEXR
  if (fmt == "exr")
    return new ExrImageSink (filename, width, height, params);
#endif
#ifdef HAVE_LIBPNG
  if (fmt == "png")
    return new PngImageSink (filename, width, height, params);
#endif
#ifdef HAVE_LIBJPEG
  if (fmt == "jpeg" || fmt == "jpg")
    return new JpegImageSink (filename, width, height, params);
#endif
#ifdef HAVE_LIBNETPBM
  if (fmt == "ppm")
    return new PpmImageSink (filename, width, height, params);
#endif

  throw std::runtime_error ("Unknown or unsupported output image type");
}



ImageSource *
ImageSource::open (const std::string &filename, const ValTable &params)
{
  std::string fmt = find_format (params, filename);

  // Formats we always support
  //
  if (fmt == "pfm")
    return new PfmImageSource (filename, params);
  else if (fmt == "rgbe" || fmt == "hdr" || fmt == "pic")
    return new RgbeImageSource (filename, params);
  else if (fmt == "tga" || fmt == "targa")
    return new TgaImageSource (filename, params);

  // Formats which are only supported if an appropriate library  is available
  //
#ifdef HAVE_LIBEXR
  if (fmt == "exr")
    return new ExrImageSource (filename, params);
#endif
#ifdef HAVE_LIBPNG
  if (fmt == "png")
    return new PngImageSource (filename, params);
#endif
#ifdef HAVE_LIBJPEG
  if (fmt == "jpeg" || fmt == "jpg")
    return new JpegImageSource (filename, params);
#endif
#ifdef HAVE_LIBNETPBM
  if (fmt == "ppm")
    return new PpmImageSource (filename, params);
#endif

  throw std::runtime_error ("Unknown or unsupported input image type");
}


// arch-tag: df36e3bf-7e23-4f22-91a3-03a954777784
