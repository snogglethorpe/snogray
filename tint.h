// tint.h -- Tint is color + alpha channel
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

#ifndef __TINT_H__
#define __TINT_H__

#include "color.h"


namespace snogray {


// A "tint" is a color with an added alpha (opacity) channel.
//
// A "pre-multiplied alpha" representation is used, meaning each color
// component implicitly reflects the alpha value.  This makes
// calculations much simpler (the downside is precision, but we have
// plenty).
//
class Tint : public Color
{
public:

  typedef Color::component_t alpha_t;

  // As with Color, the default constructor doesn't initialize anything
  // (see the Color::Color comment for more detail).
  //
  Tint () { }

  Tint (const Tint &tint) : Color (tint), alpha (tint.alpha) { }

  // For our constructor, we accept anything convertible to a color.
  //
  template<typename T>
  Tint (const T &col, alpha_t _alpha = 1)
    : Color (col * _alpha), alpha (_alpha)
  { }

  // Addition doesn't necessarily make much sense for a tint in general,
  // but is useful for accumulating samples.
  //
  Tint &operator+= (const Tint &tint)
  {
    Color::operator+= (tint);
    alpha += tint.alpha;
    return *this;
  }

  // Multiplication by a scalar.
  //
  Tint operator* (float scale) const	{ return mul (scale); }
  Tint operator* (double scale) const	{ return mul (scale); }
  Tint operator* (int scale) const	{ return mul (scale); }
  //Tint operator* (unsigned scale) const { return mul (scale); }
  Tint &operator*= (float scale)	{ return mul_assn (scale); }
  Tint &operator*= (double scale)	{ return mul_assn (scale); }
  Tint &operator*= (int scale)		{ return mul_assn (scale); }
  Tint &operator*= (unsigned scale)	{ return mul_assn (scale); }

  // Division by a scalar.
  //
  Tint operator/ (float denom) const	{ return div (denom); }
  Tint operator/ (double denom) const	{ return div (denom); }
  Tint operator/ (int denom) const	{ return div (denom); }
  Tint operator/ (unsigned denom) const { return div (denom); }
  Tint &operator/= (float denom)	{ return div_assn (denom); }
  Tint &operator/= (double denom)	{ return div_assn (denom); }
  Tint &operator/= (int denom)		{ return div_assn (denom); }
  Tint &operator/= (unsigned denom)	{ return div_assn (denom); }

  Tint clamp (float max_intens) const
  {
    Tint rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = min ((*this)[c], max_intens);
    rval.alpha = alpha;
    return rval;
  }
  Color clamp (float min_intens, float max_intens) const
  {
    Tint rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = min (max ((*this)[c], min_intens), max_intens);
    rval.alpha = alpha;
    return rval;
  }

  alpha_t alpha;

private:

  template<typename T>
  Tint mul (T scale) const
  {
    // The following loop is basically the Color operator* definition,
    // manually expanded.  Just invoking the existing definition doesn't
    // seem to generate good code, probably because the compiler gets
    // confused passing temporary objects around.

    Tint rval;
    for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
      rval[c] = (*this)[c] * scale;

    rval.alpha = alpha * scale;

    return rval;
  }

  template<typename T>
  Tint &mul_assn (T scale)
  {
    Color::operator*= (scale);
    alpha *= scale;
    return *this;
  }

  template<typename T>
  Tint div (T denom) const
  {
    return operator* (1 / component_t (denom));
  }

  template<typename T>
  Tint &div_assn (T denom)
  {
    return operator*= (1 / component_t (denom));
  }
};


}

#endif // __TINT_H__

// arch-tag: 1177d068-efe9-45fb-a517-379d07bd3609
