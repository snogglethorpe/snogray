// pos-io.cc -- Debugging output for Pos type
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <ostream>

#include "nice-io.h"

#include "pos-io.h"

std::ostream&
snogray::operator<< (std::ostream &os, const Pos &pos)
{
  os << "pos{";
  output_nicely (os, pos.x);
  os << ", ";
  output_nicely (os, pos.y);
  os << ", ";
  output_nicely (os, pos.z);
  os << "}";
  return os;
}
