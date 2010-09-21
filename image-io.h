// image-io.h -- Low-level image input and output
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_IO_H__
#define __IMAGE_IO_H__

#include <string>
#include <vector>

#include "snogmath.h"
#include "tint.h"
#include "val-table.h"


namespace snogray {

// A row in an image.
//
class ImageRow : std::vector<Tint>
{
public:

  ImageRow (unsigned _width = 0)
    : std::vector<Tint> (_width, Tint (0, 0)), width (_width)
  { }

  // Set the value of all elements in this row to zero.
  //
  void clear ()
  {
    for (unsigned i = 0; i < width; i++)
      (*this)[i] = Tint (0, 0);
  }

  // Set the width of this row to W, without initializing any new elements.
  //
  void resize (unsigned w)
  {
    std::vector<Tint>::resize (w);
    width = w;
  }

  // Set the width of this row to W, initializing any new elements to INIT_VAL.
  //
  void resize (unsigned w, const Tint &init_val)
  {
    std::vector<Tint>::resize (w, init_val);
    width = w;
  }

  // Inherit some stuff from vector
  //
  using std::vector<Tint>::operator[];
  using std::vector<Tint>::size;
  using std::vector<Tint>::at;

  unsigned width;
};


// Common superclass for ImageSink and ImageSource.  Mainly provides space
// to hold various fields.
//
class ImageIo
{
public:

  // A description of how rows are ordered in an image file, from the
  // first row in the file to the last.  The values returned are
  // integer row indices, where 0 represents the top of the image, and
  // HEIGHT-1 the bottom.
  //
  enum RowOrder { FIRST_ROW_AT_TOP, FIRST_ROW_AT_BOTTOM };

  // An object describing the row indices of the first and last (in
  // read/write order) rows in an image file, where 0 is the index of
  // the top row of the image, and HEIGHT-1 is the index of the bottom
  // row in the image.  A RowIndices::iterator can be used to
  // incrementally yield the row-indices in file read/write order.
  //
  struct RowIndices
  {
    class iterator
    {
    public:

      iterator (const RowIndices &_indices, int index)
	: indices (_indices), cur (index)
      { }

      iterator &operator++ ()
      {
	cur += (indices.first < indices.last ? 1 : -1);
	return *this;
      }
      iterator operator++ (int)
      {
	iterator old (indices, cur); ++*this; return old;
      }
      iterator operator+ (int amount) const
      {
	int offs = indices.first < indices.last ? amount : -amount;
	return iterator (indices, cur + offs);
      }

      int operator* () const { return cur; }
      bool operator== (const iterator &it) const { return cur == it.cur; }
      bool operator!= (const iterator &it) const { return cur != it.cur; }

    private:

      const RowIndices &indices;
      int cur;
    };

    RowIndices (int _first, int _last) : first (_first), last (_last) { }

    iterator begin () const { return iterator (*this, first); }
    iterator end () const { return iterator (*this, last) + 1; }

    int first, last;
  };

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

  virtual ~ImageIo () { }

  // Return true if we have an alpha (opacity) channel.
  //
  virtual bool has_alpha_channel () const { return false; }  // by default, no

  // Return the row-order of this image file.
  // (The default is top-to-bottom order, and subclasses should
  // override it as necessary)
  //
  virtual RowOrder row_order () const { return FIRST_ROW_AT_TOP; }

  // Return an object describing the row indices of the first and last
  // (in read/write order) rows in this image file, where 0 is the
  // index of the top row of the image, and HEIGHT-1 is the index of
  // the bottom row in the image.
  //
  RowIndices row_indices () const
  {
    if (row_order () == FIRST_ROW_AT_TOP)
      return RowIndices (0, height - 1);
    else
      return RowIndices (height - 1, 0);
  }

  // Handy functions to throw an error.  We use C strings instead of
  // std::string because most uses of this function pass constant strings
  // and std::string can create amazingly bloated code.
  //
  void err (const std::string &msg, bool use_errno = false)
    __attribute__ ((noreturn));
  void err (const char *msg, bool use_errno = false)
    __attribute__ ((noreturn));
  void open_err (const char *dir, const std::string &msg,
		 bool use_errno = false)
    __attribute__ ((noreturn));

  std::string filename;

  unsigned width, height;
};


// Image output

class ImageSink : public ImageIo
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
  void open_err (const std::string &msg, bool use_errno = false)
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
  void open_err (const std::string &msg, bool use_errno = false)
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
