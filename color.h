// color.h -- Color datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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
#include <cmath>

#include "rand.h"
#include "snogmath.h"

namespace Snogray {

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

  static const Color black, white, funny;

  Color () : _r (0), _g (0), _b (0) { }
  Color (component_t r, component_t g, component_t b)
    : _r (r), _g (g), _b (b)
  { }
  template<typename S> Color (S grey) : _r (grey), _g (grey), _b (grey) { }

  friend Color operator* (const ColorOnly &col1, const Color &filter);

  template<typename S>
  Color operator/ (S denom) const;

  template<typename S>
  void operator/= (S denom) { *this *= 1 / component_t (denom); }

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

  float intensity () const { return (_r + _g + _b) / 3; }

  Color clamp (float max_intens) const
  {
    return Color (std::min (_r, max_intens),
		  std::min (_g, max_intens),
		  std::min (_b, max_intens));
  }
  Color clamp (float min_intens, float max_intens) const
  {
    return Color (std::min (std::max (_r, min_intens), max_intens),
		  std::min (std::max (_g, min_intens), max_intens),
		  std::min (std::max (_b, min_intens), max_intens));
  }

  Color pow (float exp) const
  {
    return Color (powf (_r, exp), powf (_g, exp), powf (_b, exp));
  }

  component_t min_component () const
  {
    return std::min (_r, std::min (_g, _b));
  }
  component_t max_component () const
  {
    return std::min (_r, std::min (_g, _b));
  }

  component_t r () const { return _r; }
  component_t g () const { return _g; }
  component_t b () const { return _b; }

  void set_rgb (component_t r, component_t g, component_t b)
  {
    _r = r; _g = g; _b = b;
  }

  component_t _r, _g, _b;
};


// This trivial wrapper for Color is intended for use as a type where the
// implicit casting between Color and scalar values is undesirable.
//
class ColorOnly : public Color
{
public:

  ColorOnly (const Color &col) : Color (col) { }
};


template<typename S>
inline Color Color::operator/ (S denom) const
{
  return *this * (1 / component_t (denom));
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
inline Color operator/ (const ColorOnly &col1, const ColorOnly &filter)
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
  return Color (powf (base._r, exp._r),
		powf (base._g, exp._g),
		powf (base._b, exp._b));
}
inline Color log (const ColorOnly &col)
{
  return Color (logf (col._r), logf (col._g), logf (col._b));
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
  return Color (std::abs (col._r), std::abs (col._g), std::abs (col._b));
}

inline Color max (const ColorOnly &c1, const ColorOnly &c2)
{
  return Color (std::max (c1._r, c2._r),
		std::max (c1._g, c2._g),
		std::max (c1._b, c2._b));
}

inline Color min (const ColorOnly &c1, const ColorOnly &c2)
{
  return Color (std::min (c1._r, c2._r),
		std::min (c1._g, c2._g),
		std::min (c1._b, c2._b));
}

extern std::ostream& operator<< (std::ostream &os,
				 const Snogray::ColorOnly &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
