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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "color.h"

namespace Snogray {

class ImageRow
{
public:
  ImageRow (unsigned _width) : width (_width), pixels (new Color[_width]) { }

  Color& operator[] (unsigned index) { return pixels[index]; }
  const Color& operator[] (unsigned index) const { return pixels[index]; }

  Color *pixels;
  unsigned width;
};

class ImageSink
{
public:
  virtual ~ImageSink () = 0;
  virtual void write_row (const ImageRow &row) = 0;
};

class ImageSinkParams
{
public:
  ImageSinkParams (unsigned _width, unsigned _height)
    : width (_width), height (_height)
  { }
    
  virtual ImageSink *make_sink () const = 0;

  unsigned width, height;
};

class ImageOutput
{
public:
  ImageOutput (const class ImageSinkParams &params,
	       unsigned aa_factor = 1, unsigned aa_overlap = 0,
	       float (*aa_filter)(int, unsigned) = aa_box_filter);
  ~ImageOutput ();

  // Returns next row for storing into, after writing previous rows to
  // output sink.
  ImageRow &next_row ();

  unsigned aa_factor;

  // Anti-aliasing filters
  static float aa_box_filter (int offs, unsigned size);
  static float aa_triang_filter (int offs, unsigned size);
  static float aa_gauss_filter (int offs, unsigned size);

private:
  void write_accumulated_rows ();

  float *make_aa_kernel (float (*aa_filter)(int offs, unsigned size),
			 unsigned kernel_size);
  void fill_aa_row ();

  ImageSink *sink;

  ImageRow **recent_rows;
  ImageRow *aa_row;

  unsigned next_row_offs;
  unsigned num_accumulated_rows;

  float *aa_kernel;
  unsigned aa_kernel_size;
};

}

#endif /* __IMAGE_H__ */

// arch-tag: 43784b62-1eae-4938-a451-f4fdfb7db5bc
