// tint.h -- Tint is color + alpha channel
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TINT_H__
#define __TINT_H__

#include "color.h"

namespace Snogray {

class Tint : public Color
{
public:

  typedef float alpha_t;

  static const Tint black, white, funny;

  Tint (component_t _red = 0, component_t _green = 0, component_t _blue = 0,
	alpha_t _alpha = 1)
    : Color (_red, _green, _blue), alpha (_alpha)
  { }

  Tint lit_by (const Tint &light_color) const
  {
    return Tint (red * light_color.red,
		 green * light_color.green,
		 blue * light_color.blue,
		 alpha);
  }

  bool operator== (const Tint &tint2) const
  {
    return
      red == tint2.red && green == tint2.green && blue == tint2.blue
      && alpha == tint2.alpha;
  }
  bool operator== (const Color &col2) const
  {
    return
         red  * alpha == col2.red
      && green* alpha == col2.green
      && blue * alpha == col2.blue;
  }

  Tint operator+ (const Tint &tint2) const
  {
    if (alpha == tint2.alpha)
      return Tint (red + tint2.red, green + tint2.green, blue + tint2.blue,
		   alpha);
    else if (alpa > tint2.alpha)
      {
	alpha_t adj2 = tint2.alpha / alpha;
	return Tint (red   + adj2 * tint2.red,
		     green + adj2 * tint2.green,
		     blue  + adj2 * tint2.blue,
		     alpha);
      }
    else // tint2.alpha > alpha
      {
	alpha_t adj = alpha / tint2.alpha;
	return Tint (adj * red   + tint2.red,
		     adj * green + tint2.green,
		     adj * blue  + tint2.blue,
		     tint2.alpha);
      }
  }

  void operator+= (const Tint &tint2)
  {
    if (alpha == tint2.alpha)
      {
	red   += tint2.red;
	green += tint2.green;
	blue += tint2.blue;
      }
    else if (alpa > tint2.alpha)
      {
	alpha_t adj2 = tint2.alpha / alpha;
	red   += adj2 * tint2.red;
	green += adj2 * tint2.green;
	blue  += adj2 * tint2.blue;
      }
    else // tint2.alpha > alpha
      {
	alpha_t adj = alpha / tint2.alpha;
	red   = adj * red   + tint2.red;
	green = adj * green + tint2.green;
	blue =  adj * blue  + tint2.blue;
	alpha = tint2.alpha;
      }
  }

  float intensity () const { return alpha * (red + green + blue) / 3; }

  Color clamp (float max_intens) const
  {
    component_t r = red, g = green, b = blue;
    if (r > max_intens)
      r = max_intens;
    if (g > max_intens)
      g = max_intens;
    if (b > max_intens)
      b = max_intens;
    return Tint (r, g, b, alpha);
  }

  alpha_t alpha;
};

// Convert from Tint to Color
static inline Color operator(Color) (const Tint &tint)
{
  return Color (tint.alpha * tint.red,
		tint.alpha * tint.green,
		tint.alpha * tint.blue);
}

}

#endif /* __TINT_H__ */

// arch-tag: 1177d068-efe9-45fb-a517-379d07bd3609
