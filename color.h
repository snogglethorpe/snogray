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

  Color (component_t _red, component_t _green, component_t _blue)
    : red (_red), green (_green), blue (_blue)
  { }
  Color (component_t grey = 0) : red (grey), green (grey), blue (grey) { }
  Color (double grey) : red (grey), green (grey), blue (grey) { }
  Color (int grey) : red (grey), green (grey), blue (grey) { }

  friend bool operator== (const Color &col1, const Color &col2);
  friend Color operator+ (const Color &col1, const Color &col2);
  friend Color operator- (const Color &col1, const Color &col2);
  friend Color operator* (const Color &col1, const Color &filter);

  Color operator/ (float denom) const
  {
    return Color (red / denom, green / denom, blue / denom);
  }

  void operator+= (const Color &col2)
  {
    red += col2.red;
    green += col2.green;
    blue += col2.blue;
  }
  void operator-= (const Color &col2)
  {
    red -= col2.red;
    green -= col2.green;
    blue -= col2.blue;
  }
  void operator*= (float scale)
  {
    red *= scale;
    green *= scale;
    blue *= scale;
  }
  void operator/= (float denom)
  {
    red /= denom;
    green /= denom;
    blue /= denom;
  }

  float intensity () const { return (red + green + blue) / 3; }

  Color clamp (float max_intens) const
  {
    component_t r = red, g = green, b = blue;
    if (r > max_intens)
      r = max_intens;
    if (g > max_intens)
      g = max_intens;
    if (b > max_intens)
      b = max_intens;
    return Color (r, g, b);
  }
  Color pow (float exp) const
  {
    return Color (powf (red, exp),
		  powf (green, exp),
		  powf (blue, exp));
  }

  component_t red, green, blue;
};

inline bool operator== (const Color &col1, const Color &col2)
{
  return col1.red   == col2.red
      && col1.green == col2.green
      && col1.blue  == col2.blue;
}
inline bool operator!= (const Color &col1, const Color &col2)
{
  return ! operator== (col1, col2);
}

inline Color operator+ (const Color &col1, const Color &col2)
{
  return Color (col1.red   + col2.red,
		col1.green + col2.green,
		col1.blue  + col2.blue);
}
inline Color operator- (const Color &col1, const Color &col2)
{
  return Color (col1.red   - col2.red,
		col1.green - col2.green,
		col1.blue  - col2.blue);
}
inline Color operator* (const Color &col1, const Color &filter)
{
  return Color (col1.red   * filter.red,
		col1.green * filter.green,
		col1.blue  * filter.blue);
}

extern std::ostream& operator<< (std::ostream &os, const Snogray::Color &col);

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
