// tuple-matrix.h -- Matrices of tuples of floating-point numbers
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

#include "ref.h"
#include "val-table.h"
#include "color.h"


namespace snogray {

// This is the low-level storage class for tuple matrices, holding a
// matrix of floating-point values, grouped into fixed-length tuples.
//
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
		   unsigned border = 0);
  TupleMatrixData (unsigned _tuple_len, const std::string &filename,
		   const ValTable &params, unsigned border = 0);

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
  void set_pixel (unsigned x, unsigned y, const Color &col);

  // Load tuple matrix from the file FILENAME.  PARAMS contains various
  // tuple-format-specific parameters that might be needed.  The loaded
  // tuple is surrounded by a black border BORDER pixels wide.
  //
  void load (const std::string &filename, const ValTable &params,
	     unsigned border = 0);

  // Save the tuple matrix to the file FILENAME.  PARAMS contains
  // various tuple-format-specific parameters that might be needed.
  //
  void save (const std::string &filename, const ValTable &params = ValTable::NONE)
    const;

  // Store a value of type T into a tuple (assuming the tuple length is
  // correct).
  //
  // For otherwise unknown types, we assume they're a class, and try to use
  // for a "store" method which they should define.
  //
  // For some common non-class scalar types we just convert to a float and
  // store that.
  //
  template<typename T>
  void store (unsigned x, unsigned y, const T &val)
  {
    val.store (tuple (x, y));
  }
  void store (unsigned x, unsigned y, float val) { *tuple (x, y) = val; }
  void store (unsigned x, unsigned y, double val) { *tuple (x, y) = val; }
  void store (unsigned x, unsigned y, int val) { *tuple (x, y) = val; }
  void store (unsigned x, unsigned y, unsigned val) { *tuple (x, y) = val; }

  template<typename T>
  T load (unsigned x, unsigned y) const
  {
    return T (tuple (x, y));
  }

  // Number of elements in each tuple tuple; should be greater than 0.
  //
  const unsigned tuple_len;

  // The width and height of the tuple matrix.
  //
  unsigned width, height;

protected:

  // For otherwise unknown types, we assume they're a class, and try to use
  // for a static const field "TUPLE_LEN" they should define.
  //
  // For some common non-class scalar types we just return 1.
  //
  template<typename T>
  inline unsigned type_tuple_len () { return T::TUPLE_LEN; }

private:

  std::vector<float> data;

};

template<>
inline float
TupleMatrixData::load<float> (unsigned x, unsigned y) const
{
  return *tuple (x, y);
}
template<>
inline double
TupleMatrixData::load<double> (unsigned x, unsigned y) const
{
  return *tuple (x, y);
}
template<>
inline int
TupleMatrixData::load<int> (unsigned x, unsigned y) const
{
  return int (*tuple (x, y));
}
template<>
inline unsigned
TupleMatrixData::load<unsigned> (unsigned x, unsigned y) const
{
  return unsigned (*tuple (x, y));
}

template<>
inline unsigned TupleMatrixData::type_tuple_len<float> () { return 1; }
template<>
inline unsigned TupleMatrixData::type_tuple_len<int> () { return 1; }
template<>
inline unsigned TupleMatrixData::type_tuple_len<double> () { return 1; }



// This is the high-level tuple-matrix class, a matrix of tuples of type T.
//
template<class T>
class TupleMatrix : public TupleMatrixData
{
public:

  TupleMatrix (unsigned _width, unsigned _height)
    : TupleMatrixData (type_tuple_len<T> (), _width, _height)
  { }

  // Constructors for a matrix loaded from an image file.
  //
  TupleMatrix (const std::string &filename, unsigned border = 0)
    : TupleMatrixData (type_tuple_len<T> (), filename, border)
  { }
  TupleMatrix (const std::string &filename, const ValTable &params,
	       unsigned border = 0)
    : TupleMatrixData (type_tuple_len<T> (), filename, params, border)
  { }

  // Constructor for extracting a sub-matrix of BASE.  If W or H are 0, the
  // maximum available width or height is used.  Note that because of the
  // defaults, this is used as the copy-constructor.
  //
  TupleMatrix (const TupleMatrix &base,
	       unsigned offs_x = 0, unsigned offs_y = 0,
	       unsigned w = 0, unsigned h = 0)
    : TupleMatrixData (type_tuple_len<T> (), base, offs_x, offs_y, w, h)
  { }
  TupleMatrix (const Ref<TupleMatrix> &base,
	       unsigned offs_x = 0, unsigned offs_y = 0,
	       unsigned w = 0, unsigned h = 0)
    : TupleMatrixData (type_tuple_len<T> (), *base, offs_x, offs_y, w, h)
  { }

  T operator() (unsigned x, unsigned y) const
  {
    return get (x, y);
  }

  T get (unsigned x, unsigned y) const
  {
    return load<T> (x, y);
  }

  void put (unsigned x, unsigned y, const T &val)
  {
    store (x, y, val);
  }

};

}

#endif /* __TUPLE_MATRIX_H__ */

// arch-tag: 810abe16-816a-4fe0-a417-ab25c4f306ac
