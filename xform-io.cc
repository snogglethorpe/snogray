// xform-io.cc -- Debugging output for Xform type
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

#include "xform-io.h"

std::ostream&
snogray::operator<< (std::ostream &os, const Xform &xform)
{
  os << "xform<";

  for (int row = 0; row < 4; row++)
    {
      if (row > 0)
	os << ";  ";
      for (int col = 0; col < 4; col++)
	{
	  if (col > 0)
	    os << ", ";
	  output_nicely (os, xform (col, row));
	}
    }

  os << ">";

  return os;
}
