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

namespace Snogray {

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

  friend Color operator* (const Color &col1, const Color &filter);
  friend Color operator/ (const Color &col1, const Color &filter);

  template<typename S>
  Color operator/ (S denom) { return *this * (1 / component_t (denom)); }
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
    _r /= filter._r;
    _g /= filter._g;
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

inline bool operator== (const Color &col1, const Color &col2)
{
  return col1._r == col2._r && col1._g == col2._g && col1._b == col2._b;
}
inline bool operator!= (const Color &col1, const Color &col2)
{
  return ! operator== (col1, col2);
}

inline bool operator> (const Color &col1, const Color &col2)
{
  return (col1._r + col1._g + col1._b) > (col2._r + col2._g + col2._b);
}
inline bool operator<= (const Color &col1, const Color &col2)
{
  return ! operator> (col1, col2);
}

inline bool operator< (const Color &col1, const Color &col2)
{
  return (col1._r + col1._g + col1._b) < (col2._r + col2._g + col2._b);
}
inline bool operator>= (const Color &col1, const Color &col2)
{
  return ! operator< (col1, col2);
}


inline Color operator+ (const Color &col1, const Color &col2)
{
  return Color (col1._r + col2._r, col1._g + col2._g, col1._b + col2._b);
}
inline Color operator- (const Color &col1, const Color &col2)
{
  return Color (col1._r - col2._r, col1._g - col2._g, col1._b - col2._b);
}
inline Color operator* (const Color &col1, const Color &filter)
{
  return Color (col1._r * filter._r, col1._g * filter._g, col1._b * filter._b);
}
inline Color operator/ (const Color &col1, const Color &filter)
{
  return Color (col1._r / filter._r, col1._g / filter._g, col1._b / filter._b);
}

inline Color pow (const Color &base, const Color &exp)
{
  return Color (powf (base._r, exp._r),
		powf (base._g, exp._g),
		powf (base._b, exp._b));
}
inline Color log (const Color &col)
{
  return Color (logf (col._r), logf (col._g), logf (col._b));
}

inline Color random (Color limit = 1.f)
{
  return Color (random (limit._r), random (limit._g), random (limit._b));
}

inline Color random (Color min, Color limit)
{
  return Color (random (min._r, limit._r),
		random (min._g, limit._g),
		random (min._b, limit._b));
}


extern std::ostream& operator<< (std::ostream &os, const Snogray::Color &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
