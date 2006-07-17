// image-png.h -- PNG format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PNG_H__
#define __IMAGE_PNG_H__

#include <cstdio>

#include <libpng/png.h>

#include "image-byte-vec.h"

namespace Snogray {

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
		const Params &params = Params::NONE);
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
		  const Params &params = Params::NONE);
  ~PngImageSource ();

  virtual void read_row (ByteVec &byte_vec);

private:

  png_structp libpng_struct;
  png_infop libpng_info;

  FILE *stream;
};

}

#endif /* __IMAGE_PNG_H__ */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
