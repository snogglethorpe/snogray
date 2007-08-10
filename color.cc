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

#include <iomanip>

#include "color.h"
#include "coords.h" // for lim()

using namespace snogray;


Color::Color (component_t r, component_t g, component_t b)
{
  // XXX
  _components[0] = r;
  _components[1] = g;
  _components[2] = b;
}

Color::component_t
Color::r () const
{
  return _components[0]; // XXX
}

Color::component_t
Color::g () const
{
  return _components[1]; // XXX
}

Color::component_t
Color::b () const
{
  return _components[2]; // XXX
}

void
Color::set_rgb (component_t r, component_t g, component_t b)
{
  // XXX
  _components[0] = r;
  _components[1] = g;
  _components[2] = b;
}

std::ostream&
snogray::operator<< (std::ostream &os, const Color &col)
{
  os << "color<";
  os << std::setprecision (5) << lim (col[0]);
  for (unsigned c = 1; c < Color::NUM_COMPONENTS; c++)
    os << ", " << std::setprecision (5) << lim (col[c]);
  os << '>';
  return os;
}

// arch-tag: 11e71f8e-3323-473e-95ce-e3e07e6197d8
