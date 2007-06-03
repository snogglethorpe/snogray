// color.h -- Color datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COLOR_H__
#define __COLOR_H__

#include <fstream>

#include "rand.h"
#include "snogmath.h"

namespace snogray {

class ColorOnly;

// A color.
//
// This class is currently defined in terms of R/G/B components, but
// designed to be used abstractly for the most part (in the future, some
// other representation might be used).  To this end, many standard math
// operators work on colors directly (usually treating each component
// independently).
//
class Color
{
public:

  typedef float component_t;

  // Tuple length for reading and writing images.
  //
  static const unsigned TUPLE_LEN = 3;

  static const Color black, white, funny;

  Color () : _r (0), _g (0), _b (0) { }
  Color (component_t r, component_t g, component_t b)
    : _r (r), _g (g), _b (b)
  { }

  // Scalar constructors
  //
  Color (int grey) : _r (grey), _g (_r), _b (_r) { }
  Color (float grey) : _r (grey), _g (_r), _b (_r) { }
  Color (double grey) : _r (grey), _g (_r), _b (_r) { }
  // template<typename S> Color (S grey) : _r (grey), _g (grey), _b (grey) { }

  // Constructor for making from an image tuple; there must be at least
  // TUPLE_LEN floats available at tuple.
  //
  explicit Color (const float tuple[])
    : _r (tuple[0]), _g (tuple[1]), _b (tuple[2])
  { }

  // Store this color into the floating-poing tuple TUPLE; there must be
  // at least TUPLE_LEN floats available at tuple.
  //
  void store (float tuple[]) const
  {
    tuple[0] = _r; tuple[1] = _g; tuple[2] = _b;
  }

  const Color &operator+= (const Color &col2)
  {
    _r += col2._r;
    _g += col2._g;
    _b += col2._b;
    return *this;
  }
  void operator-= (const Color &col2)
  {
    _r -= col2._r;
    _g -= col2._g;
    _b -= col2._b;
  }
  void operator*= (const Color &filter)
  {
    _r *= filter._r;
    _g *= filter._g;
    _b *= filter._b;
  }
  void operator/= (const Color &filter)
  {
    if (filter._r != 0)
      _r /= filter._r;
    if (filter._g != 0)
      _g /= filter._g;
    if (filter._b != 0)
      _b /= filter._b;
  }

  // Doesn't make much sense physically, of course, but useful for some
  // formulas.
  //
  Color operator- () const { return Color (-_r, -_g, -_b); }

  friend Color operator* (const ColorOnly &col1, const Color &filter);

  // Division by a scalar.
  //
  Color operator/ (component_t denom) const;
  void operator/= (component_t denom) { *this *= 1 / denom; }

  float intensity () const { return (_r + _g + _b) / 3; }

  Color clamp (float max_intens) const
  {
    return Color (min (_r, max_intens),
		  min (_g, max_intens),
		  min (_b, max_intens));
  }
  Color clamp (float min_intens, float max_intens) const
  {
    return Color (min (max (_r, min_intens), max_intens),
		  min (max (_g, min_intens), max_intens),
		  min (max (_b, min_intens), max_intens));
  }

  Color pow (float exp) const
  {
    return Color (std::pow (_r, exp), std::pow (_g, exp), std::pow (_b, exp));
  }

  component_t min_component () const
  {
    return min (_r, min (_g, _b));
  }
  component_t max_component () const
  {
    return min (_r, min (_g, _b));
  }

  component_t r () const { return _r; }
  component_t g () const { return _g; }
  component_t b () const { return _b; }

  void set_rgb (component_t r, component_t g, component_t b)
  {
    _r = r; _g = g; _b = b;
  }

  // Array access to color components.
  //
  component_t &operator[] (unsigned c) { return _components[c]; }
  const component_t &operator[] (unsigned c) const { return _components[c]; }

  // We use an anonymous union to allow both "array" and "named
  // component" access to the color components.
  //
  union
  {
    // Named components.
    //
    struct { component_t _r, _g, _b; };

    // Array components.
    //
    component_t _components[TUPLE_LEN];
  };
};


// This trivial wrapper for Color is intended for use as a type where the
// implicit casting between Color and scalar values is undesirable.
//
class ColorOnly : public Color
{
public:

  ColorOnly (const Color &col) : Color (col) { }
};


// Division by a scalar.
//
inline Color
Color::operator/ (component_t denom) const
{
  return *this * (1 / denom);
}


// We define two-operand operations such that the first argument must be a
// color, but the second may be a color or a scalar implicitly cast to a
// color.  This avoids most ambiguity problems between these definitions
// and the standard scalar definitions, while still allowing fairly natural
// usage.  [There are a few exceptions, to match specific usage.]

inline bool operator== (const ColorOnly &col1, const Color &col2)
{
  return col1._r == col2._r && col1._g == col2._g && col1._b == col2._b;
}
inline bool operator!= (const ColorOnly &col1, const Color &col2)
{
  return ! operator== (col1, col2);
}

inline bool operator> (const ColorOnly &col1, const Color &col2)
{
  return (col1._r + col1._g + col1._b) > (col2._r + col2._g + col2._b);
}
inline bool operator<= (const ColorOnly &col1, const Color &col2)
{
  return ! operator> (col1, col2);
}

inline bool operator< (const ColorOnly &col1, const Color &col2)
{
  return (col1._r + col1._g + col1._b) < (col2._r + col2._g + col2._b);
}
inline bool operator>= (const ColorOnly &col1, const Color &col2)
{
  return ! operator< (col1, col2);
}


inline Color operator+ (const ColorOnly &col1, const Color &col2)
{
  return Color (col1._r + col2._r, col1._g + col2._g, col1._b + col2._b);
}
inline Color operator- (const Color &col1, const ColorOnly &col2)
{
  return Color (col1._r - col2._r, col1._g - col2._g, col1._b - col2._b);
}
inline Color operator* (const ColorOnly &col1, const Color &filter)
{
  return Color (col1._r * filter._r, col1._g * filter._g, col1._b * filter._b);
}
inline Color operator/ (const Color &col1, const ColorOnly &filter)
{
  return Color (filter._r == 0 ? 0 : col1._r / filter._r,
		filter._g == 0 ? 0 : col1._g / filter._g,
		filter._b == 0 ? 0 : col1._b / filter._b);
}

// Multiplication by a float is very common (for scaling operations), so we
// provide a `float * Color' operator too.
//
inline Color operator* (float scale, const ColorOnly &col)
{
  return Color (scale * col._r, scale * col._g, scale * col._b);
}


inline Color pow (const ColorOnly &base, const Color &exp)
{
  return Color (pow (base._r, exp._r),
		pow (base._g, exp._g),
		pow (base._b, exp._b));
}
inline Color log (const ColorOnly &col)
{
  return Color (log (col._r), log (col._g), log (col._b));
}

inline Color random (const ColorOnly &limit = Color (1.f))
{
  return Color (random (limit._r), random (limit._g), random (limit._b));
}

inline Color random (const ColorOnly &min, const ColorOnly &limit)
{
  return Color (random (min._r, limit._r),
		random (min._g, limit._g),
		random (min._b, limit._b));
}

inline Color abs (const ColorOnly &col)
{
  return Color (abs (col._r), abs (col._g), abs (col._b));
}

inline Color sqrt (const ColorOnly &col)
{
  return Color (sqrt (col._r), sqrt (col._g), sqrt (col._b));
}

inline Color max (const ColorOnly &c1, const ColorOnly &c2)
{
  return Color (max (c1._r, c2._r), max (c1._g, c2._g), max (c1._b, c2._b));
}

inline Color min (const ColorOnly &c1, const ColorOnly &c2)
{
  return Color (min (c1._r, c2._r), min (c1._g, c2._g), min (c1._b, c2._b));
}

extern std::ostream& operator<< (std::ostream &os,
				 const snogray::ColorOnly &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
