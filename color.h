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

  float intensity () const { return (r + g + b) / 3; }

  Color clamp (float max_intens) const
  {
    component_t _r = r, _g = g, _b = b;
    if (_r > max_intens)
      _r = max_intens;
    if (_g > max_intens)
      _g = max_intens;
    if (_b > max_intens)
      _b = max_intens;
    return Color (_r, _g, _b);
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

extern std::ostream& operator<< (std::ostream &os, const Snogray::Color &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
