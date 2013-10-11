// color-io.cc -- Debugging output for Color type
//
//  Copyright (C) 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <ostream>

#include "util/nice-io.h"
#include "color.h"

#include "color-io.h"


std::ostream&
snogray::operator<< (std::ostream &os, const Color &col)
{
  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    {
      os << ((c == 0) ? "color{" : ", ");
      output_nicely (os, col[c]);
    }
  os << '}';

  return os;
}
