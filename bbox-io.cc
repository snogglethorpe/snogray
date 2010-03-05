// bbox-io.cc -- Debugging output for BBox type
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

#include "nice-io.h"

#include "bbox-io.h"

std::ostream&
snogray::operator<< (std::ostream &os, const BBox &bbox)
{
  os << "bbox<";
  output_nicely (os, bbox.min.x);
  os << ", ";
  output_nicely (os, bbox.min.y);
  os << ", ";
  output_nicely (os, bbox.min.z);
  os << " - ";
  output_nicely (os, bbox.max.x);
  os << ", ";
  output_nicely (os, bbox.max.y);
  os << ", ";
  output_nicely (os, bbox.max.z);
  os << ">";
  return os;
}
