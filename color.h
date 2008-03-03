// color.h -- Color datatype
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

#ifndef __COLOR_H__
#define __COLOR_H__

#include <fstream>

#include "rand.h"
#include "snogmath.h"


namespace snogray {


// A color.
//
// Abstract color type.  This is mostly an opaque type, and as many
// standard math operators work on colors directly (usually treating each
// component independently), a Color can usually be treated like the
// traditional C numeric types.
//
class Color
{
public:

  // Type of one color component.
  //
  typedef float component_t;

  // Number of color components stored.
  //
  static const unsigned NUM_COMPONENTS = 3; // RGB

  // Tuple length for reading and writing images.
  //
  static const unsigned TUPLE_LEN = NUM_COMPONENTS;

  // Default constructor.
  //
  // Note that by default, the color-components are _not initialized_!
  // This make code generation in much more efficient some cases (where
  // color temporaries are declared and then later assigned to, the
  // compiler isn't always smart enough to get rid of the initialization of
  // the temporary).
  //
  // In general Colors should be treated like traditional C scalar types,
  // and can just be initialized with zero where desired.
  //
  Color () {}

  // RGB constructor.  This constructor is Deprecated:  It is only used
  // in some test-scene and image-loading code, and should be replaced by
  // some more proper color-space conversion.
  //
  Color (component_t r, component_t g, component_t b);

  // Scalar constructors; these are very handy for mixing colors and
  // scalars in expressions.
  //
  Color (int grey)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] = grey;
  }
  Color (float grey)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] = grey;
  }
  Color (double grey)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] = grey;
  }

  // Constructor for making from an image tuple; there must be at least
  // TUPLE_LEN floats available at tuple.
  //
  explicit Color (const float tuple[])
  {
    for (unsigned c = 0; c < TUPLE_LEN; c++)
      _components[c] = tuple[c];
  }

  // Store this color into the floating-poing tuple TUPLE; there must be
  // at least TUPLE_LEN floats available at tuple.
  //
  void store (float tuple[]) const
  {
    for (unsigned c = 0; c < TUPLE_LEN; c++)
      tuple[c] = _components[c];
  }

  const Color &operator+= (const Color &col2)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] += col2[c];
    return *this;
  }
  void operator-= (const Color &col2)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] -= col2[c];
  }
  void operator*= (const Color &filter)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] *= filter[c];
  }
  void operator/= (const Color &filter)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      if (filter[c] != 0)
	_components[c] /= filter[c];
  }

  // Explicit handling of scaling (results in better code generation
  // than relying on implicit promotion of the scale argument to Color).
  // We enumerate all common scalar types to try and avoid overload
  // ambiguity.
  void operator*= (float scale)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] *= scale;
  }
  void operator*= (double scale)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] *= scale;
  }
  void operator*= (int scale)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] *= scale;
  }
  void operator*= (unsigned scale)
  {
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      _components[c] *= scale;
  }

  // Doesn't make much sense physically, of course, but useful for some
  // formulas.
  //
  Color operator- () const
  {
    Color rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = -_components[c];
    return rval;
  }

  // Division by a scalar.
  //
  Color operator/ (float denom) const;
  Color operator/ (double denom) const;
  Color operator/ (int denom) const;
  Color operator/ (unsigned denom) const;
  void operator/= (float denom)    { *this *= 1 / component_t (denom); }
  void operator/= (double denom)   { *this *= 1 / component_t (denom); }
  void operator/= (int denom)      { *this *= 1 / component_t (denom); }
  void operator/= (unsigned denom) { *this *= 1 / component_t (denom); }

  component_t intensity () const
  {
    component_t sum = _components[0];
    for (unsigned c = 1; c < NUM_COMPONENTS; c++)
      sum += _components[c];
    return sum / NUM_COMPONENTS;
  }

  Color clamp (float max_intens) const
  {
    Color rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = min (_components[c], max_intens);
    return rval;
  }
  Color clamp (float min_intens, float max_intens) const
  {
    Color rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = min (max (_components[c], min_intens), max_intens);
    return rval;
  }

  Color pow (component_t exp) const
  {
    Color rval;
    for (unsigned c = 0; c < NUM_COMPONENTS; c++)
      rval[c] = std::pow (_components[c], exp);
    return rval;
  }

  component_t min_component () const
  {
    component_t min_comp = _components[0];
    for (unsigned c = 1; c < NUM_COMPONENTS; c++)
      min_comp = min (_components[c], min_comp);
    return min_comp;
  }
  component_t max_component () const
  {
    component_t max_comp = _components[0];
    for (unsigned c = 1; c < NUM_COMPONENTS; c++)
      max_comp = max (_components[c], max_comp);
    return max_comp;
  }

  // Array access to color components.
  //
  component_t &operator[] (unsigned c) { return _components[c]; }
  const component_t &operator[] (unsigned c) const { return _components[c]; }

  // Getter and setter methods for RGB primaries.  These methods are
  // deprecated:  They are only used by some image I/O code, and should be
  // replaced by some more proper color-space conversion.
  //
  component_t r () const;
  component_t g () const;
  component_t b () const;
  //
  void set_rgb (component_t r, component_t g, component_t b);

private:

  // Array components.
  //
  component_t _components[NUM_COMPONENTS];
};


inline bool operator== (const Color &col1, const Color &col2)
{
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    if (col1[c] != col2[c])
      return false;
  return true;
}
inline bool operator!= (const Color &col1, const Color &col2)
{
  return ! operator== (col1, col2);
}

inline bool operator> (const Color &col1, const Color &col2)
{
  Color::component_t sum1 = col1[0], sum2 = col2[0];
  for (unsigned c = 1; c < Color::NUM_COMPONENTS; c++)
    {
      sum1 += col1[c];
      sum2 += col2[c];
    }
  return sum1 > sum2;
}
inline bool operator<= (const Color &col1, const Color &col2)
{
  return ! operator> (col1, col2);
}

inline bool operator< (const Color &col1, const Color &col2)
{
  Color::component_t sum1 = col1[0], sum2 = col2[0];
  for (unsigned c = 1; c < Color::NUM_COMPONENTS; c++)
    {
      sum1 += col1[c];
      sum2 += col2[c];
    }
  return sum1 < sum2;
}
inline bool operator>= (const Color &col1, const Color &col2)
{
  return ! operator< (col1, col2);
}


inline Color operator+ (const Color &col1, const Color &col2)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col1[c] + col2[c];
  return rval;
}
inline Color operator- (const Color &col1, const Color &col2)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col1[c] - col2[c];
  return rval;
}

inline Color operator* (const Color &col1, const Color &filter)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col1[c] * filter[c];
  return rval;
}
inline Color operator/ (const Color &col1, const Color &filter)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = (filter[c] == 0) ? 0 : col1[c] / filter[c];
  return rval;
}

// Multiplication by scalars is very common (for scaling operations), so
// provide explicit overloads for it, which helps the compiler.  We
// enumerate all common scalar types to try and avoid overload ambiguity.
//
inline Color operator* (const Color &col, float scale)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}
inline Color operator* (float scale, const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}
inline Color operator* (const Color &col, double scale)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}
inline Color operator* (double scale, const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}
inline Color operator* (const Color &col, int scale)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}
inline Color operator* (int scale, const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = col[c] * scale;
  return rval;
}

// Similarly for division by a scalar.
//
inline Color
Color::operator/ (float denom) const
{
  return *this * (1 / component_t (denom));
}
inline Color
Color::operator/ (double denom) const
{
  return *this * (1 / component_t (denom));
}
inline Color
Color::operator/ (int denom) const
{
  return *this * (1 / component_t (denom));
}
inline Color
Color::operator/ (unsigned denom) const
{
  return *this * (1 / component_t (denom));
}

inline Color pow (const Color &base, const Color &exp)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = pow (base[c], exp[c]);
  return rval;
}
inline Color log (const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = log (col[c]);
  return rval;
}

inline Color fmod (const Color &col1, const Color &col2)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = fmod (col1[c], col2[c]);
  return rval;
}

inline Color random (const Color &limit = Color (1.f))
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = random (limit[c]);
  return rval;
}

inline Color random (const Color &min, const Color &limit)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = random (min[c], limit[c]);
  return rval;
}

inline Color abs (const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = abs (col[c]);
  return rval;
}

inline Color sqrt (const Color &col)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = sqrt (col[c]);
  return rval;
}

inline Color max (const Color &col1, const Color &col2)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = max (col1[c], col2[c]);
  return rval;
}

inline Color min (const Color &col1, const Color &col2)
{
  Color rval;
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    rval[c] = min (col1[c], col2[c]);
  return rval;
}

extern std::ostream& operator<< (std::ostream &os, const snogray::Color &col);


}

#endif /* __COLOR_H__ */


// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
