// tint.h -- Tint is color + alpha channel
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

namespace snogray {

class Tint : public Color
{
public:

  typedef float alpha_t;

  static const Tint black, white, funny;

  Tint (component_t _r = 0, component_t _g = 0, component_t _b = 0,
	alpha_t _a = 1)
    : Color (_r, _g, _b), a (_a)
  { }

  Tint lit_by (const Tint &light_color) const
  {
    return Tint (r * light_color.r, g * light_color.g, b * light_color.b, a);
  }

  bool operator== (const Tint &tint2) const
  {
    return r == tint2.r && g == tint2.g && b == tint2.b && a == tint2.a;
  }
  bool operator== (const Color &col2) const
  {
    return r * a == col2.r && g * a == col2.g && b * a == col2.b;
  }

  Tint operator+ (const Tint &tint2) const
  {
    if (a == tint2.a)
      return Tint (r + tint2.r, g + tint2.g, b + tint2.b, a);
    else if (alpa > tint2.a)
      {
	a_t adj2 = tint2.a / a;
	return Tint (r + adj2 * tint2.r, g + adj2 * tint2.g, b + adj2 * tint2.b,
		     a);
      }
    else // tint2.a > a
      {
	a_t adj = a / tint2.a;
	return Tint (adj * r + tint2.r, adj * g + tint2.g, adj * b + tint2.b,
		     tint2.a);
      }
  }

  void operator+= (const Tint &tint2)
  {
    if (a == tint2.a)
      {
	r += tint2.r;
	g += tint2.g;
	b += tint2.b;
      }
    else if (alpa > tint2.a)
      {
	alpha_t adj2 = tint2.a / a;
	r += adj2 * tint2.r;
	g += adj2 * tint2.g;
	b += adj2 * tint2.b;
      }
    else // tint2.a > a
      {
	alpha_t adj = a / tint2.a;
	r = adj * r + tint2.r;
	g = adj * g + tint2.g;
	b = adj * b + tint2.b;
	a = tint2.a;
      }
  }

  float intensity () const { return a * (r + g + b) / 3; }

  Color clamp (float max_intens) const
  {
    component_t _r = r, _g = g, _b = b;
    if (_r > max_intens)
      _r = max_intens;
    if (_g > max_intens)
      _g = max_intens;
    if (_b > max_intens)
      _b = max_intens;
    return Tint (_r, _g, _b, a);
  }

  alpha_t a;
};

// Convert from Tint to Color
static inline Color operator(Color) (const Tint &tint)
{
  return Color (tint.a * tint.r, tint.a * tint.g, tint.a * tint.b);
}

}

#endif /* __TINT_H__ */

// arch-tag: 1177d068-efe9-45fb-a517-379d07bd3609
