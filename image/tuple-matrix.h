// tuple-matrix.h -- Generic matrix storage type
//
//  Copyright (C) 2005-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TUPLE_MATRIX_H
#define SNOGRAY_TUPLE_MATRIX_H

#include "config.h"

#include <vector>

#include "ref.h"
#include "color/color.h"
#include "val-table.h"
#include "tuple-adaptor.h"

// Use OpenEXR "half" datatype as default matrix storage element if possible.
//
#if HAVE_LIBEXR
# include <half.h>
#endif

namespace snogray {


class ImageInput;
class ImageScaledOutput;

// Use OpenEXR "half" datatype for default matrix storage element if possible.
//
#if HAVE_LIBEXR
typedef half default_tuple_element_type;
#else
typedef float default_tuple_element_type;
#endif



// This is the low-level storage class for tuple matrices, holding a matrix
// of "data" values of type DT, grouped into fixed-length tuples.
//
template<typename DT = default_tuple_element_type>
class TupleMatrixData : public RefCounted
{
public:

  TupleMatrixData (unsigned _tuple_len, unsigned _width, unsigned _height)
    : tuple_len (_tuple_len), width (_width), height (_height),
      data (_tuple_len * _width * _height)
  { }

  // Constructors for a matrix loaded from an image file.
  //
  TupleMatrixData (unsigned _tuple_len, const std::string &filename,
		   const ValTable &params = ValTable::NONE);
  TupleMatrixData (unsigned _tuple_len, ImageInput &src,
		   const ValTable &params = ValTable::NONE);

  // Constructor for extracting a sub-matrix of BASE.  If W or H are 0,
  // the maximum available width or height is used.  Note that because
  // of the defaults, this is used as the copy-constructor.
  //
  TupleMatrixData (unsigned _tuple_len, const TupleMatrixData &base,
		   unsigned offs_x = 0, unsigned offs_y = 0,
		   unsigned w = 0, unsigned h = 0);

  // Set all elements of this matrix to DT(0).  Note that the
  // TupleMatrixData constructor may _not_ do this, as it uses DT's
  // default constructor, and that may not do any initialization at
  // all (this is true, for instance, of scalar types like float).
  //
  void zero ()
  {
    size_t len = data.size ();
    for (size_t i = 0; i < len; i++)
      data[i] = DT (0);
  }

  // Return a pointer to the first element of the tuple at location X, Y
  //
  const DT *tuple (unsigned x, unsigned y) const
  {
    return &data[(y * width + x) * tuple_len];
  }
  DT *tuple (unsigned x, unsigned y)
  {
    return &data[(y * width + x) * tuple_len];
  }

  // Load tuple matrix from the file FILENAME.  PARAMS contains
  // various tuple-format-specific parameters that might be needed.
  //
  void load (const std::string &filename,
	     const ValTable &params = ValTable::NONE);

  // Load tuple matrix from the image input SRC.  PARAMS contains
  // various tuple-format-specific parameters that might be needed.
  //
  void load (ImageInput &src, const ValTable &params = ValTable::NONE);

  // Save the tuple matrix to the file FILENAME.  PARAMS contains
  // various tuple-format-specific parameters that might be needed.
  //
  void save (const std::string &filename,
	     const ValTable &params = ValTable::NONE)
    const;

  // Save the tuple matrix to the output OUT.  PARAMS contains various
  // tuple-format-specific parameters that might be needed.
  //
  void save (ImageScaledOutput &out, const ValTable &params = ValTable::NONE)
    const;

  // Number of elements in each tuple tuple; should be greater than 0.
  //
  const unsigned tuple_len;

  // The width and height of the tuple matrix.
  //
  const unsigned width, height;

private:

  // Return a color from the tuple at location X, Y; if the tuple length is
  // not the same as a Color, only Color::TUPLE_LEN members are returned,
  // and any missing components set to zero.
  //
  Color pixel (unsigned x, unsigned y) const;

  // Set the tuple at location X, Y from the color COL; if the tuple length
  // is not the same as a Color, only the first TUPLE_LEN members are
  // copied, and any missing components set to zero.
  //
  void set_pixel (unsigned x, unsigned y, const Color &col);

  std::vector<DT> data;
};



// This is the high-level tuple-matrix class, a matrix of values of type T.
//
template<typename T, typename DT = default_tuple_element_type>
class TupleMatrix : public TupleMatrixData<DT>
{
public:

  typedef TupleAdaptor<T, DT> TA;
  typedef TupleMatrixData<DT> TMD;

  TupleMatrix (unsigned _width, unsigned _height)
    : TMD (TA::TUPLE_LEN, _width, _height)
  { }

  // Constructors for a matrix loaded from an image file.
  //
  TupleMatrix (const std::string &filename,
	       const ValTable &params = ValTable::NONE)
    : TMD (TA::TUPLE_LEN, filename, params)
  { }
  TupleMatrix (ImageInput &src, const ValTable &params = ValTable::NONE)
    : TMD (TA::TUPLE_LEN, src, params)
  { }

  // Constructor for extracting a sub-matrix of BASE.  If W or H are 0, the
  // maximum available width or height is used.  Note that because of the
  // defaults, this is used as the copy-constructor.
  //
  TupleMatrix (const TupleMatrix &base,
	       unsigned offs_x = 0, unsigned offs_y = 0,
	       unsigned w = 0, unsigned h = 0)
    : TMD (TA::TUPLE_LEN, base, offs_x, offs_y, w, h)
  { }
  TupleMatrix (const Ref<TupleMatrix> &base,
	       unsigned offs_x = 0, unsigned offs_y = 0,
	       unsigned w = 0, unsigned h = 0)
    : TMD (TA::TUPLE_LEN, *base, offs_x, offs_y, w, h)
  { }

  TupleAdaptor<T, const DT> operator() (unsigned x, unsigned y) const
  {
    return TupleAdaptor<T, const DT> (TupleMatrix<T,DT>::tuple (x, y));
  }
  TupleAdaptor<T, DT> operator() (unsigned x, unsigned y)
  {
    return TupleAdaptor<T, DT> (TupleMatrix<T,DT>::tuple (x, y));
  }

};

}


// Include method definitions
//
#include "tuple-matrix.tcc"


#endif /* SNOGRAY_TUPLE_MATRIX_H */

// arch-tag: 810abe16-816a-4fe0-a417-ab25c4f306ac
