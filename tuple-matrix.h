// tuple-matrix.h -- Matrices of tuples of floating-point numbers
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TUPLE_MATRIX_H__
#define __TUPLE_MATRIX_H__

#include <vector>

#include "params.h"
#include "color.h"


namespace Snogray {

// This is the low-level storage class for tuple matrices, holding a
// matrix of floating-point values, grouped into fixed-length tuples.
//
class TupleMatrixData
{
public:

  TupleMatrixData (unsigned _tuple_len, unsigned _width, unsigned _height)
    : tuple_len (_tuple_len), width (_width), height (_height),
      data (_tuple_len * _width * _height)
  { }

  // Constructors for a matrix loaded from an image file.
  //
  TupleMatrixData (unsigned _tuple_len, const std::string &filename,
		   unsigned border = 0);
  TupleMatrixData (unsigned _tuple_len, const std::string &filename,
		   const Params &params, unsigned border = 0);

  // Constructor for extracting a sub-matrix of BASE.  If W or H are 0,
  // the maximum available width or height is used.  Note that because
  // of the defaults, this is used as the copy-constructor.
  //
  TupleMatrixData (unsigned _tuple_len, const TupleMatrixData &base,
		   unsigned offs_x = 0, unsigned offs_y = 0,
		   unsigned w = 0, unsigned h = 0);

  // Return a pointer to the first element of the tuple at location X, Y
  //
  const float *tuple (unsigned x, unsigned y) const
  {
    return &data[(y * width + x) * tuple_len];
  }
  float *tuple (unsigned x, unsigned y)
  {
    return &data[(y * width + x) * tuple_len];
  }

  // Return a color from the tuple at location X, Y (if the tuple length is
  // not three, an appropriate translation is done).  This mapping of
  // tuples to colors is mainly used for doing I/O to image files.
  //
  Color pixel (unsigned x, unsigned y) const;

  // Set the tuple at location X, Y from the color COL (if the tuple
  // length is not three, an appropriate translation is done).
  //
  void put (unsigned x, unsigned y, const Color &col);

  // Load tuple matrix from the file FILENAME.  PARAMS contains various
  // tuple-format-specific parameters that might be needed.  The loaded
  // tuple is surrounded by a black border BORDER pixels wide.
  //
  void load (const std::string &filename, const Params &params,
	     unsigned border = 0);

  // Save the tuple matrix to the file FILENAME.  PARAMS contains
  // various tuple-format-specific parameters that might be needed.
  //
  void save (const std::string &filename, const Params &params = Params::NONE)
    const;

  // Number of elements in each tuple tuple; should be greater than 0.
  //
  const unsigned tuple_len;

  // The width and height of the tuple matrix.
  //
  unsigned width, height;

private:

  std::vector<float> data;
};



// This is the high-level tuple-matrix class, a matrix of tuples of type T.
//
template<class T>
class TupleMatrix : public TupleMatrixData
{
public:

  TupleMatrix (unsigned _width, unsigned _height)
    : TupleMatrixData (T::TUPLE_LEN, _width, _height)
  { }

  // Constructors for a matrix loaded from an image file.
  //
  TupleMatrix (const std::string &filename, unsigned border = 0)
    : TupleMatrixData (T::TUPLE_LEN, filename, border)
  { }
  TupleMatrix (const std::string &filename, const Params &params,
	       unsigned border = 0)
    : TupleMatrixData (T::TUPLE_LEN, filename, params, border)
  { }

  // Constructor for extracting a sub-matrix of BASE.  If W or H are 0, the
  // maximum available width or height is used.  Note that because of the
  // defaults, this is used as the copy-constructor.
  //
  TupleMatrix (const TupleMatrix<T> &base,
	       unsigned offs_x = 0, unsigned offs_y = 0,
	       unsigned w = 0, unsigned h = 0)
    : TupleMatrixData (T::TUPLE_LEN, base, offs_x, offs_y, w, h)
  { }

  T operator() (unsigned x, unsigned y) const
  {
    return get (x, y);
  }

  T get (unsigned x, unsigned y) const
  {
    return T (tuple (x, y));
  }
  void put (unsigned x, unsigned y, const T &val)
  {
    val.store (tuple (x, y));
  }

};

}

#endif /* __TUPLE_MATRIX_H__ */

// arch-tag: 810abe16-816a-4fe0-a417-ab25c4f306ac
