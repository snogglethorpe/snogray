// vec-io.cc -- Debugging output for Vec type
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

#include "vec-io.h"

std::ostream&
snogray::operator<< (std::ostream &os, const Vec &vec)
{
  os << "vec{";
  output_nicely (os, vec.x);
  os << ", ";
  output_nicely (os, vec.y);
  os << ", ";
  output_nicely (os, vec.z);
  os << "}";
  return os;
}
