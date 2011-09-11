// image-png.h -- PNG format image handling
//
//  Copyright (C) 2005, 2006, 2007, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_PNG_H
#define SNOGRAY_IMAGE_PNG_H

#include <cstdio>

#include <libpng/png.h>

#include "image-byte-vec.h"

namespace snogray {

class PngErrState
{
public:

  PngErrState (const std::string &_filename)
    : libpng_err (false), err_filename (_filename)
  { }

  void throw_libpng_err ();

  static void libpng_err_handler (png_structp libpng_struct, const char *msg);

  bool libpng_err;
  std::string libpng_err_msg;

  // Just reference to the filename stored elsewhere.
  //
  const std::string &err_filename;
};

class PngImageSink : public ByteVecImageSink, PngErrState
{  
public:

  PngImageSink (const std::string &filename,
		unsigned width, unsigned height,
		const ValTable &params = ValTable::NONE);
  ~PngImageSink ();

  virtual void write_row (const ByteVec &byte_vec);

  // Write previously written rows to disk, if possible.  This may flush
  // I/O buffers etc., but will not in any way change the output (so for
  // instance, it will _not_ flush the compression state of a PNG output
  // image, as that can make the resulting compression worse).
  //
  virtual void flush ();

private:

  png_structp libpng_struct;
  png_infop libpng_info;

  FILE *stream;
};

class PngImageSource : public ByteVecImageSource, PngErrState
{  
public:

  PngImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE);
  ~PngImageSource ();

  virtual void read_row (ByteVec &byte_vec);

private:

  png_structp libpng_struct;
  png_infop libpng_info;

  FILE *stream;
};

}

#endif /* SNOGRAY_IMAGE_PNG_H */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
