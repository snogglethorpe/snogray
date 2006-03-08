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

namespace Snogray {

class Color
{
public:
  typedef float component_t;

  static const Color black, white, funny;

  Color () : r (0), g (0), b (0) { }
  Color (component_t _r, component_t _g, component_t _b)
    : r (_r), g (_g), b (_b)
  { }
  template<typename S> Color (S grey) : r (grey), g (grey), b (grey) { }

  friend Color operator* (const Color &col1, const Color &filter);
  friend Color operator/ (const Color &col1, const Color &filter);

  template<typename S>
  Color operator/ (S denom) { return *this * (1 / component_t (denom)); }
  template<typename S>
  void operator/= (S denom) { *this *= 1 / component_t (denom); }

  void operator+= (const Color &col2)
  {
    r += col2.r;
    g += col2.g;
    b += col2.b;
  }
  void operator-= (const Color &col2)
  {
    r -= col2.r;
    g -= col2.g;
    b -= col2.b;
  }
  void operator*= (const Color &filter)
  {
    r *= filter.r;
    g *= filter.g;
    b *= filter.b;
  }
  void operator/= (const Color &filter)
  {
    r /= filter.r;
    g /= filter.g;
    b /= filter.b;
  }

  // Doesn't make much sense physically, of course, but useful for some
  // formulas.
  //
  Color operator- () const { return Color (-r, -g, -b); }

  float intensity () const { return (r + g + b) / 3; }

  Color clamp (float max_intens) const
  {
    return Color (std::min (r, max_intens),
		  std::min (g, max_intens),
		  std::min (b, max_intens));
  }
  Color clamp (float min_intens, float max_intens) const
  {
    return Color (std::min (std::max (r, min_intens), max_intens),
		  std::min (std::max (g, min_intens), max_intens),
		  std::min (std::max (b, min_intens), max_intens));
  }

  Color pow (float exp) const
  {
    return Color (powf (r, exp), powf (g, exp), powf (b, exp));
  }

  component_t r, g, b;
};

inline bool operator== (const Color &col1, const Color &col2)
{
  return col1.r == col2.r && col1.g == col2.g && col1.b == col2.b;
}
inline bool operator!= (const Color &col1, const Color &col2)
{
  return ! operator== (col1, col2);
}

inline bool operator> (const Color &col1, const Color &col2)
{
  return (col1.r + col1.g + col1.b) > (col2.r + col2.g + col2.b);
}
inline bool operator<= (const Color &col1, const Color &col2)
{
  return ! operator> (col1, col2);
}

inline bool operator< (const Color &col1, const Color &col2)
{
  return (col1.r + col1.g + col1.b) < (col2.r + col2.g + col2.b);
}
inline bool operator>= (const Color &col1, const Color &col2)
{
  return ! operator< (col1, col2);
}


inline Color operator+ (const Color &col1, const Color &col2)
{
  return Color (col1.r + col2.r, col1.g + col2.g, col1.b + col2.b);
}
inline Color operator- (const Color &col1, const Color &col2)
{
  return Color (col1.r - col2.r, col1.g - col2.g, col1.b - col2.b);
}
inline Color operator* (const Color &col1, const Color &filter)
{
  return Color (col1.r * filter.r, col1.g * filter.g, col1.b * filter.b);
}
inline Color operator/ (const Color &col1, const Color &filter)
{
  return Color (col1.r / filter.r, col1.g / filter.g, col1.b / filter.b);
}

inline Color pow (const Color &base, const Color &exp)
{
  return Color (powf (base.r, exp.r),
		powf (base.g, exp.g),
		powf (base.b, exp.b));
}
inline Color log (const Color &col)
{
  return Color (logf (col.r), logf (col.g), logf (col.b));
}


extern std::ostream& operator<< (std::ostream &os, const Snogray::Color &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
