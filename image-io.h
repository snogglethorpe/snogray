// image-io.h -- Low-level image input and output
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
#include <vector>

#include "snogmath.h"
#include "color.h"
#include "val-table.h"


namespace snogray {

// A row in an image.
//
class ImageRow : std::vector<Color>
{
public:

  ImageRow (unsigned _width = 0)
    : std::vector<Color> (_width, 0), width (_width)
  { }

  void clear () { for (unsigned i = 0; i < width; i++) (*this)[i] = 0; }
  void resize (unsigned w) { std::vector<Color>::resize (w); width = w; }

  // Inherit some stuff from vector
  //
  using std::vector<Color>::operator[];
  using std::vector<Color>::size;
  using std::vector<Color>::at;

  unsigned width;
};


// Common superclass for ImageSink and ImageSource.  Mainly provides space
// to hold various fields.
//
class ImageIo
{
public:

  // If FILENAME has a recognized extension from which we can guess its
  // format, return it (converted to lower-case).
  //
  static std::string filename_format (const std::string &filename);

  // Return true if FILENAME has a recogized image format we can read.
  //
  static bool recognized_filename (const std::string &filename);

  // If PARAMS contains an explicit "format" entry, return its value,
  // otherwise if FILENAME has a recognized extension from which we can
  // guess its format, return it (converted to lower-case).
  //
  static std::string find_format (const ValTable &params,
				  const std::string &filename);


  ImageIo (const std::string &_filename, unsigned _width, unsigned _height)
    : filename (_filename), width (_width), height (_height)
  { }

  // Handy functions to throw an error.  We use C strings instead of
  // std::string because most uses of this function pass constant strings
  // and std::string can create amazingly bloated code.
  //
  void err (const char *msg, bool use_errno = false)
    __attribute__ ((noreturn));
  void open_err (const char *dir, const char *msg = "", bool use_errno = false)
    __attribute__ ((noreturn));

  std::string filename;

  unsigned width, height;
};


// Image output

class ImageSink  : public ImageIo
{
public:

  static const float DEFAULT_TARGET_GAMMA = 2.2;

  static ImageSink *open (const std::string &filename,
			  unsigned width, unsigned height,
			  const ValTable &params = ValTable::NONE);

  virtual ~ImageSink () = 0;

  virtual void write_row (const ImageRow &row) = 0;

  // Write previously written rows to disk, if possible.  This may flush
  // I/O buffers etc., but will not in any way change the output (so for
  // instance, it will _not_ flush the compression state of a PNG output
  // image, as that can make the resulting compression worse).
  //
  virtual void flush ();

  virtual float max_intens () const;

  void open_err (const char *msg = "", bool use_errno = false)
    __attribute__ ((noreturn));

protected:

  ImageSink (const std::string &filename, unsigned width, unsigned height,
	     const ValTable &)
    : ImageIo (filename, width, height)
  { }
};


// Image input

class ImageSource : public ImageIo
{
public:

  static ImageSource *open (const std::string &filename,
			    const ValTable &params = ValTable::NONE);

  virtual ~ImageSource ();

  void open_err (const char *msg = "", bool use_errno = false)
    __attribute__ ((noreturn));

  virtual void read_row (ImageRow &row) = 0;

protected:

  ImageSource (const std::string &filename, const ValTable &)
    : ImageIo (filename, 0, 0)
  { }
};

}

#endif /* __IMAGE_IO_H__ */

// arch-tag: 43784b62-1eae-4938-a451-f4fdfb7db5bc
