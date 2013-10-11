// tint-io.cc -- Debugging output for Tint type
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
#include "tint.h"

#include "tint-io.h"


std::ostream&
snogray::operator<< (std::ostream &os, const Tint &tint)
{
  Color col = tint.alpha_scaled_color ();

  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
    {
      os << ((c == 0) ? "tint<" : ", ");
      output_nicely (os, col[c]);
    }
  os << ", a=" << tint.alpha;
  os << '>';

  return os;
}
