// color.h -- Color datatype
//
//  Copyright (C) 2005-2007, 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "color.h"


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


// arch-tag: 11e71f8e-3323-473e-95ce-e3e07e6197d8
