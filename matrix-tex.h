// matrix-tex.h -- 2d texture based on discrete matrix of values
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_TEX_H__
#define __MATRIX_TEX_H__

#include <string>

#include "snogmath.h"
#include "tex.h"
#include "tuple-matrix.h"
#include "matrix-linterp.h"


namespace snogray {


// A 2d texture based on a matrix tuple (probably loaded from an image).
//
template<typename T>
class MatrixTex : public Tex<T>
{
public:

  MatrixTex (const std::string &filename);

  // This constructor stores a (ref-counted) reference to CONTENTS.
  //
  MatrixTex (const Ref<TupleMatrix<T> > &contents);

  // This constructor _copies_ the specified region of BASE (and so
  // doesn't reference BASE).
  //
  MatrixTex (const TupleMatrix<T> &base,
	     unsigned offs_x, unsigned offs_y, unsigned w, unsigned h);

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const;


private:

  template<class MT>
  struct Iter
  {
    Iter (MT &_mat, unsigned x = 0, unsigned y = 0)
      : mat (_mat), _x (x), _y (y)
    { }

    // This is intended for normal-to-const iterator conversion.
    //
    template<class MT2>
    Iter (const Iter<MT2> &i2) : mat (i2.mat), _x (i2._x), _y (i2._y) { }

    Iter &operator++ ()
    {
      if (++_x == mat.matrix->width)
	{
	  _x = 0;
	  ++_y;
	}
      return *this;
    }

    bool operator== (const Iter &i2) const { return _x == i2._x && _y == i2._y; }
    bool operator!= (const Iter &i2) const { return _x != i2._x || _y != i2._y; }

    UV uv () const { return mat.interp.map (_x, _y); }

    unsigned x () const { return _x; }
    unsigned y () const { return _y; }

    // Returns the value of the texture where the iterator points;
    // slightly faster than doing a normal texture lookup.
    //
    T val () const { return mat.matrix->get (_x, _y); }

    void set (const T &val) const { mat.matrix->put (_x, _y, val); }

    MT &mat;

    unsigned _x, _y;
  };

  friend class iterator;
  friend class const_iterator;

public:

  typedef Iter<MatrixTex> iterator;
  typedef Iter<const MatrixTex> const_iterator;

  iterator begin () { return iterator (*this); }
  const_iterator begin () const { return const_iterator (*this); }

  const_iterator end () const {return const_iterator(*this, 0, matrix->height);}

  // Matrix holding data for this texture.
  //
  Ref<TupleMatrix<T> > matrix;

private:

  const MatrixLinterp interp;
};


} // namespace snogray

// Include method definitions
//
#include "matrix-tex.tcc"

#endif // __MATRIX_TEX_H__


// arch-tag: 0d92b346-d347-4025-aab0-cbaf594f6789
