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

class PngImageSink : public ByteVecImageSink
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

  FILE *stream;

  png_structp png;
  png_infop png_info;
};

class PngImageSource : public ByteVecImageSource
{  
public:

  PngImageSource (const std::string &filename,
		  const Params &params = Params::NONE);
  ~PngImageSource ();

  virtual void read_row (ByteVec &byte_vec);

private:

  FILE *stream;

  png_structp png;
  png_infop png_info;
};

}

#endif /* __IMAGE_PNG_H__ */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
