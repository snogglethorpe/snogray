// image-ppm.h -- PPM format image handling
//
//  Copyright (C) 2005-2007, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_PPM_H
#define SNOGRAY_IMAGE_PPM_H

extern "C" {
#include <ppm.h>
#undef min
#undef max
#undef abs
}

#include <stdexcept>

#include "image-byte-vec.h"


// PPM has fixed gamma correction:  each sample is "...  proportional to
// the intensity of the CIE Rec. 709 red, green, and blue in the pixel,
// adjusted by the CIE Rec.  709 gamma transfer function.  (That transfer
// function specifies a gamma number of 2.2 and has a linear section for
// small intensities)."
//
#define IMAGE_PPM_GAMMA 2.2

// asdlkfjasf
#define _IMAGE_PPM_GAMMA_STRING "2.2"


namespace snogray {



// Output

class PpmImageSink : public ByteVecImageSink
{
public:

  PpmImageSink (const std::string &filename, unsigned width, unsigned height,
		const ValTable &params = ValTable::NONE);
  ~PpmImageSink ();

  // Note that we override the ImageSink::read_row(ImageRow&), instead
  // of ByteVecImageSink::read_row(const ByteVec&) as is normal for
  // subclasses of ByteVecImageSink.  This because PPM has its own
  // abstraction for writing into an image row; however we still use
  // other facilities of ByteVecImageSink, such as float-to-integer
  // component conversion, etc.
  //
  virtual void write_row (const ImageRow &row);

  // This should never be called.
  //
  virtual void write_row (const ByteVec &)
  {
    throw std::runtime_error ("PpmImageSink::write_row(const ByteVec &)");
  }

  // Write previously written rows to disk, if possible.  This may flush
  // I/O buffers etc., but will not in any way change the output (so for
  // instance, it will _not_ flush the compression state of a PNG output
  // image, as that can make the resulting compression worse).
  //
  virtual void flush () { fflush (stream); }

private:

  FILE *stream;

  // A single row of bytes we use as temporary storage during output
  //
  pixel *output_row;

  pixval max_pixval;
};



// Input

class PpmImageSource : public ByteVecImageSource
{  
public:

  PpmImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE);
  ~PpmImageSource ();

  // Note that we override the ImageSource::read_row(ImageRow&),
  // instead of ByteVecImageSource::read_row(ByteVec&) as is normal
  // for subclasses of ByteVecImageSource.  This because PPM has its
  // own abstraction for reading from an image row; however we still
  // use other facilities of ByteVecImageSource, such as int-to-float
  // component conversion, etc.
  //
  virtual void read_row (ImageRow &row);

  // This should never be called.
  //
  virtual void read_row (ByteVec &)
  {
    throw std::runtime_error ("PpmImageSink::read_row(ByteVec &)");
  }

private:

  FILE *stream;

  // A single row of bytes we use as temporary storage during input
  //
  pixel *input_row;

  // PPM params
  //
  pixval max_pixval;
  int format;
};


}

#endif // SNOGRAY_IMAGE_PPM_H
