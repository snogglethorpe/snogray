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
#include <csetjmp>
#include <jpeglib.h>

#include "image-byte-vec.h"

namespace Snogray {

class JpegErrState : public jpeg_error_mgr
{
public:

  JpegErrState (const std::string &_filename);

  // This is similar to the standard C `setjmp': it should be called
  // before attempting a libjpeg operation that might yield an error,
  // and will return false; if an error subsquently happens during the
  // _following_ operation, this call will essentially return a second
  // time, this time with a return value of true.
  //
  // It must be inline, because `setjmp' uses special compiler support.
  //
  bool trap_err () { return err || setjmp (jmpbuf) != 0; }

  // If an error was seen, throw an appropriate exception.
  //
  void throw_err ();

  // True if we saw an error.
  //
  bool err;

  std::string err_msg;

  // Just reference to the filename stored elsewhere.
  //
  const std::string &err_filename;

private:

  // Used for jump from one of our error handlers back past libjpeg
  // library routines into the nearest calling C++ function.
  // Unfortunately we can't just throw an error directly from the error
  // handler, because the C++ runtime seems to abort as soon as it hits
  // a non-C++ function on the stack.
  //
  jmp_buf jmpbuf;

  // Called for fatal errors.
  //
  static void libjpeg_err_handler (j_common_ptr cinfo);

  // Called for warnings (MSG_LEVEL < 0) and "trace messages" (>= 0).
  //
  static void libjpeg_warn_handler (j_common_ptr cinfo, int msg_level);

  // Call to output a message.
  //
  static void libjpeg_msg_handler (j_common_ptr cinfo);
};

class JpegImageSink : public ByteVecImageSink
{  
public:

  static const int DEFAULT_QUALITY = 98;

  JpegImageSink (const std::string &filename,
		 unsigned width, unsigned height,
		 const Params &params = Params::NONE);
  ~JpegImageSink ();

  virtual void write_row (const ByteVec &rgb_bytes);

  // Write previously written rows to disk, if possible.  This may flush
  // I/O buffers etc., but will not in any way change the output (so for
  // instance, it will _not_ flush the compression state of a PNG output
  // image, as that can make the resulting compression worse).
  //
  virtual void flush ();

private:

  FILE *stream;

  jpeg_compress_struct jpeg_info;

  JpegErrState jpeg_err;
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

  struct JpegErrState jpeg_err;
};

}

#endif /* __IMAGE_JPEG_H__ */


// arch-tag: 354fa041-9c04-419b-a6e5-5c76fb3734cb
