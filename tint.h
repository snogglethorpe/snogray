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


// A "tint" is a color plus an alpha (opacity) channel.
//
// The color component is kept using a "pre-multiplied alpha"
// representation, meaning each color component implicitly reflects the
// alpha value.  This makes calculations much simpler (the downside is
// precision, but we have plenty).
//
class Tint
{
public:

  typedef Color::component_t component_t;
  typedef Color::component_t alpha_t;

  // As with Color, the default constructor doesn't initialize anything
  // (see the Color::Color comment for more detail).
  //
  Tint () { }

  Tint (const Tint &tint) : color (tint.color), alpha (tint.alpha) { }

  // For our constructor, we accept anything convertible to a color.
  //
  template<typename T>
  Tint (const T &col, alpha_t _alpha)
    : color (col * _alpha), alpha (_alpha)
  { }
  template<typename T>
  Tint (const T &col)
    : color (col), alpha (1)
  { }

  // Return the color portion of this tint, scaled by the alpha value.
  //
  // As that is the form Tint stores the value in, this method just
  // returns a reference to the stored value.
  //
  const Color &alpha_scaled_color () const { return color; }

  // Return the color portion of this tint, unscaled by any alpha value.
  //
  Color unscaled_color () const
  {
    if (alpha == 0)
      return 0;
    else
      return color / alpha;
  }

  // Addition doesn't necessarily make much sense for a tint in general,
  // but is useful for accumulating samples.
  //
  Tint &operator+= (const Tint &tint)
  {
    color += tint.color;
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
    rval.color = color.clamp (max_intens);
    rval.alpha = alpha;
    return rval;
  }
  Tint clamp (float min_intens, float max_intens) const
  {
    Tint rval;
    rval.color = color.clamp (min_intens, max_intens);
    rval.alpha = alpha;
    return rval;
  }

  // Inherit Color behavior for these methods.  Note that they will operate
  // on the color scaled by alpha.
  //
  component_t min_component () const { return color.min_component(); }
  component_t max_component () const { return color.max_component(); }

  // Set this tint to the color R,G,B and alpha A, where R, G, and B have
  // already been scaled by A.
  //
  void set_scaled_rgba (component_t r, component_t g, component_t b, alpha_t a)
  {
    color.set_rgb (r, g, b);
    alpha = a;
  }

  // Set this tint to the color R,G,B and alpha A, where R, G, and B have
  // have _not_ been scaled by A.
  //
  void set_unscaled_rgba (component_t r, component_t g, component_t b,
			  alpha_t a)
  {
    color.set_rgb (r * a, g * a, b * a);
    alpha = a;
  }

  // Set this tint to the color R,G,B and an alpha of 1.
  //
  void set_rgb (component_t r, component_t g, component_t b)
  {
    color.set_rgb (r, g, b);
    alpha = 1;
  }

  // The color of this tint in pre-multiplied alpha form.
  //
  Color color;

  // The alpha channel.
  //
  alpha_t alpha;

private:

  template<typename T>
  Tint mul (T scale) const
  {
    Tint rval;
    rval.color = color * scale;
    rval.alpha = alpha * scale;
    return rval;
  }

  template<typename T>
  Tint &mul_assn (T scale)
  {
    color *= scale;
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


inline Tint max (const Tint &tint1, const Tint &tint2)
{
  Tint rval;
  rval.color = max (tint1.color, tint2.color);
  rval.alpha = max (tint1.alpha, tint2.alpha);
  return rval;
}


}

#endif // __TINT_H__

// arch-tag: 1177d068-efe9-45fb-a517-379d07bd3609
