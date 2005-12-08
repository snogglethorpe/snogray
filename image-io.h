// image-io.h -- Image input and output
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_IO_H__
#define __IMAGE_IO_H__

#include <string>
#include <cmath>

#include "color.h"


namespace Snogray {

// Rows in an image

class ImageRow
{
public:
  ImageRow (unsigned _width) : width (_width), pixels (new Color[_width]) { }
  ~ImageRow () { delete[] pixels; }

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
  virtual void error (const std::string &msg) const __attribute__ ((noreturn)) = 0;

  // Calls error with current errno message appended
  void sys_error (const std::string &msg) const __attribute__ ((noreturn));

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
  static const float DEFAULT_QUALITY = 98; // 0-100

  ImageSinkParams ()
    : width (0), height (0),
      exposure (0),
      aa_factor (0), aa_overlap (0), aa_filter (0),
      target_gamma (0), quality (0)
  { }

  class ImageSink *make_sink () const;

  // This is called when something wrong is detect with some parameter
  virtual void error (const std::string &msg) const __attribute__ ((noreturn)) = 0;

  unsigned width, height;

  // The intensity of the output image is scaled by 2^exposure
  //
  float exposure;

  unsigned aa_factor, aa_overlap;
  float (*aa_filter) (int offs, unsigned size);

  // The following are for use of backends.  Not all backends use all
  // parameters.  0 means unspecified (use default/ignore).
  float target_gamma;		// For backends with limited dynamic range
  float quality;		// For lossy compression; range 0-100 ala jpeg
};

// This is the class that format-specific subclasses override; it just
// provides a sturb error method which throws an exception if called.
struct ImageFmtSinkParams : ImageSinkParams
{
  ImageFmtSinkParams (const ImageSinkParams &params)
    : ImageSinkParams (params), generic_params (&params)
  { }

  virtual void error (const std::string &msg) const __attribute__ ((noreturn));

  // Original generic parameters, if any
  const ImageSinkParams *generic_params;
};

class ImageSink
{
public:
  ImageSink (const ImageSinkParams &params) { }

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

  // The intensity of the output image is scaled by 2^exposure
  //
  float exposure;

  unsigned aa_factor;

  // Anti-aliasing filters
  static float aa_box_filter (int offs, unsigned size);
  static float aa_triang_filter (int offs, unsigned size);
  static float aa_gauss_filter (int offs, unsigned size);

private:

  void write_accumulated_rows ();

  float *make_aa_kernel (float (*aa_filter)(int offs, unsigned size),
			 unsigned kernel_size);
  ImageRow &fill_aa_row ();

  ImageSink *sink;

  float intensity_scale;	// 2^exposure

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
  virtual void error (const std::string &msg) const __attribute__ ((noreturn)) = 0;
};

// This is the class that format-specific subclasses override; it just
// provides a sturb error method which throws an exception if called.
struct ImageFmtSourceParams : ImageSourceParams
{
  ImageFmtSourceParams (const ImageSourceParams &params)
    : ImageSourceParams (params), generic_params (&params)
  { }

  virtual void error (const std::string &msg) const __attribute__ ((noreturn));

  // Original generic parameters, if any
  const ImageSourceParams *generic_params;
};

class ImageSource
{
public:
  static ImageSource *make (const std::string &filename, const char *format);
  virtual ~ImageSource ();
  virtual void read_size (unsigned &width, unsigned &height) = 0;
  virtual void read_row (ImageRow &row) = 0;
};

class ImageInput
{
public:
  ImageInput (const ImageSourceParams &params)
    : source (params.make_source ())
  {
    source->read_size (width, height);
  }
  ImageInput (const std::string &filename, const char *format)
    : source (ImageSource::make (filename, format))
  {
    source->read_size (width, height);
  }
  ~ImageInput () { delete source; }

  // Reads a row of image data into ROW
  void read_row (ImageRow &row) { source->read_row (row); }

  // Set from the image
  unsigned width, height;

private:

  ImageSource *source;
};

}

#endif /* __IMAGE_IO_H__ */

// arch-tag: 43784b62-1eae-4938-a451-f4fdfb7db5bc
