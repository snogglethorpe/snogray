// matrix-tex2.h -- 2d texture based on discrete matrix of values
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_TEX2_H__
#define __MATRIX_TEX2_H__

#include <string>

#include "uv.h"
#include "snogmath.h"
#include "tuple-matrix.h"
#include "tex2.h"


namespace snogray {


// This is a common class for doing MatrixTex2 interpolation
//
class MatrixTex2Interp
{
public:

  // Subclasses typicall do not know the proper scale until they have
  // initialized their other fields, so they should set u_scale and v_scale
  // fields later.
  //
  MatrixTex2Interp (unsigned _width, unsigned _height)
    : width (_width), height (_height),
      u_scale (width - 1), v_scale (height - 1)
  { }

  // Calculate interpolation coordinates and weights.  Subclasses must do
  // the actual interpolation.
  //
  void calc_params (tparam_t u, tparam_t v,
		    unsigned &xi_lo, unsigned &yi_lo,
		    unsigned &xi_hi, unsigned &yi_hi,
		    float &x_lo_fr, float &y_lo_fr,
		    float &x_hi_fr, float &y_hi_fr)
    const
  {
    // Remap to range [0, 1)
    //
    u -= floor (u);
    v -= floor (v);

    tparam_t   x    = u * u_scale,  y    = v * v_scale;
    tparam_t   x_lo = floor (x),    y_lo = floor (y);

    x_hi_fr = x - x_lo;
    y_hi_fr = y - y_lo;
    x_lo_fr = 1 - x_hi_fr;
    y_lo_fr = 1 - y_hi_fr;

    xi_lo = unsigned (x_lo);
    yi_lo = unsigned (y_lo);
    xi_hi = xi_lo + 1;
    yi_hi = yi_lo + 1;

    if (xi_hi >= width)
      xi_hi -= width;
    if (yi_hi >= width)
      yi_hi -= width;

    yi_lo = height - yi_lo - 1;
    yi_hi = height - yi_hi - 1;
  }

  UV map (unsigned x, unsigned y) const 
  {
    return UV (float (x) / u_scale, float (y) / v_scale);
  }
  
private:

  unsigned width, height;

  tparam_t u_scale, v_scale;
};

// A 2d texture based on a matrix tuple (probably loaded from an image).
//
template<typename T>
class MatrixTex2 : public Tex2<T>
{
public:

  MatrixTex2 (const std::string &filename);

  // This constructor stores a (ref-counted) _reference_ to BASE.
  //
  MatrixTex2 (const Ref<TupleMatrix<T> > &base);

  // This constructor _copies_ the specified region of BASE (and so doesn't
  // reference BASE).
  //
  MatrixTex2 (const TupleMatrix<T> &base,
	      unsigned offs_x, unsigned offs_y, unsigned w, unsigned h);
  MatrixTex2 (const Ref<TupleMatrix<T> > &base,
	      unsigned offs_x, unsigned offs_y, unsigned w, unsigned h);

  virtual T map (tparam_t u, tparam_t v) const;

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

  typedef Iter<MatrixTex2> iterator;
  typedef Iter<const MatrixTex2> const_iterator;

  iterator begin () { return iterator (*this); }
  const_iterator begin () const { return const_iterator (*this); }

  const_iterator end () const {return const_iterator(*this, 0, matrix->height);}

  // Matrix holding data for this texture.
  //
  Ref<TupleMatrix<T> > matrix;

private:

  const MatrixTex2Interp interp;
};


} // namespace snogray

// Include method definitions
//
#include "matrix-tex2.tcc"

#endif // __MATRIX_TEX2_H__


// arch-tag: 0d92b346-d347-4025-aab0-cbaf594f6789
