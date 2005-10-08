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

// Rows in an image

class ImageRow
{
public:
  ImageRow (unsigned _width) : width (_width), pixels (new Color[_width]) { }

  Color& operator[] (unsigned index) { return pixels[index]; }
  const Color& operator[] (unsigned index) const { return pixels[index]; }

  unsigned width;
  Color *pixels;
};



struct ImageParams
{
  ImageParams () : file_name (0), format (0) { }
  virtual ~ImageParams (); // stop gcc bitching

  // This is called when something wrong is detect with some parameter
  virtual void error (const char *msg) const = 0;

  // Return the file format to use; if the FORMAT field is 0, then try
  // to guess it from FILE_NAME.
  const char *find_format () const;

  const char *file_name;	// 0 means standard input
  const char *format;		// 0 means auto-detect
};


// Image output

struct ImageSinkParams : ImageParams
{
  static const float DEFAULT_TARGET_GAMMA = 2.2;

  ImageSinkParams ()
    : width (0), height (0), target_gamma (0),
      aa_factor (0), aa_overlap (0), aa_filter (0)
  { }

  class ImageSink *make_sink () const;

  // This is called when something wrong is detect with some parameter
  virtual void error (const char *msg) const = 0;

  unsigned width, height;

  float target_gamma;		// 0 means unspecified (use default)

  unsigned aa_factor, aa_overlap;
  float (*aa_filter) (int offs, unsigned size);
};

// This is the class that format-specific subclasses override; it just
// provides a sturb error method which throws an exception if called.
struct ImageFmtSinkParams : ImageSinkParams
{
  ImageFmtSinkParams (const ImageSinkParams &params)
    : ImageSinkParams (params)
  { }

  virtual void error (const char *msg) const;
};

class ImageSink
{
public:
  virtual ~ImageSink () = 0;
  virtual void write_row (const ImageRow &row) = 0;
  virtual float max_intens () const;
};

class ImageOutput
{
public:
  typedef float (*aa_filter_t) (int offs, unsigned size);

  static const aa_filter_t DEFAULT_AA_FILTER;

  ImageOutput (const ImageSinkParams &params);
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
  float aa_max_intens;
};


// Image input

struct ImageSourceParams : ImageParams
{
  class ImageSource *make_source () const;

  // This is called when something wrong is detect with some parameter
  virtual void error (const char *msg) const __attribute__ ((noreturn)) = 0;
};

// This is the class that format-specific subclasses override; it just
// provides a sturb error method which throws an exception if called.
struct ImageFmtSourceParams : ImageSourceParams
{
  ImageFmtSourceParams (const ImageSourceParams &params)
    : ImageSourceParams (params)
  { }

  virtual void error (const char *msg) const;
};

class ImageSource
{
public:
  virtual ~ImageSource ();
  virtual void read_size (unsigned &width, unsigned &height) = 0;
  virtual void read_row (ImageRow &row) = 0;
};

class ImageInput
{
public:
  ImageInput (const ImageSourceParams &params)
    : source (params.make_source ())
  { source->read_size (width, height); }
  ~ImageInput () { delete source; }

  // Reads a row of image data into ROW
  void read_row (ImageRow &row) { source->read_row (row); }

  // Set from the image
  unsigned width, height;

private:
  ImageSource *source;
};

}

#endif /* __IMAGE_H__ */

// arch-tag: 43784b62-1eae-4938-a451-f4fdfb7db5bc
