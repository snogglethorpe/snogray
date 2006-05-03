// image-jpeg.h -- JPEG format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_JPEG_H__
#define __IMAGE_JPEG_H__

#include <cstdio>
#include <jpeglib.h>

#include "image-byte-vec.h"

namespace Snogray {

class JpegImageSink : public ByteVecImageSink
{  
public:

  static const int DEFAULT_QUALITY = 98;

  JpegImageSink (const std::string &filename,
		 unsigned width, unsigned height,
		 const Params &params = Params::NONE);
  ~JpegImageSink ();

  virtual void write_row (const ByteVec &rgb_bytes);

private:

  FILE *stream;

  struct jpeg_compress_struct jpeg_info;
  struct jpeg_error_mgr jpeg_err;
};

class JpegImageSource : public ByteVecImageSource
{  
public:

  JpegImageSource (const std::string &filename,
		   const Params &params = Params::NONE);
  ~JpegImageSource ();

  virtual void read_row (ByteVec &rgb_bytes);

private:

  FILE *stream;

  struct jpeg_decompress_struct jpeg_info;
  struct jpeg_error_mgr jpeg_err;
};

}

#endif /* __IMAGE_JPEG_H__ */


// arch-tag: 354fa041-9c04-419b-a6e5-5c76fb3734cb
