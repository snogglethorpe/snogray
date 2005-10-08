// color.h -- Color datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

  Color (component_t _red = 0, component_t _green = 0, component_t _blue = 0)
    : red (_red), green (_green), blue (_blue)
  { }

  Color lit_by (const Color &light_color) const
  {
    return Color (red * light_color.red,
		  green * light_color.green,
		  blue * light_color.blue);
  }

  bool operator== (const Color &col2) const
  {
    return red == col2.red && green == col2.green && blue == col2.blue;
  }

  Color operator* (float scale) const
  {
    return Color (red * scale, green * scale, blue * scale);
  }
  Color operator+ (const Color &col2) const
  {
    return Color (red + col2.red, green + col2.green, blue + col2.blue);
  }
  Color operator- (const Color &col2) const
  {
    return Color (red - col2.red, green - col2.green, blue - col2.blue);
  }
  Color operator/ (float denom) const
  {
    return Color (red / denom, green / denom, blue / denom);
  }

  float intensity () const { return (red + green + blue) / 3; }

  void operator+= (const Color &col2)
  {
    red += col2.red;
    green += col2.green;
    blue += col2.blue;
  }
  void operator*= (float scale)
  {
    red *= scale;
    green *= scale;
    blue *= scale;
  }

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

static inline Color
operator* (float scale, const Color &color)
{
  return color * scale;
}

extern std::ostream& operator<< (std::ostream &os, const Snogray::Color &col);


// Colors with alpha channel

#if 0

class ColorAlpha : public Color 
{
public:
  typedef float alpha_t;

  ColorAlpha (component_t r = 0, component_t g = 0, component_t b = 0,
	      alpha_t a)
    : Color (r, g, b), alpha (a)
  { }

}

// Convert from ColorAlpha to Color by throwing away alpha channel
static inline Color operator(Color) (const ColorAlpha &cola)
{
  return Color (cola.red, cola.green, cola.blue);
}

#endif

}

#endif /* __COLOR_H__ */

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
