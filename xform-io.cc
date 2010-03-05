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

  for (int j = 0; j < 4; j++)
    {
      if (j > 0)
	os << "; ";
      for (int i = 0; i < 4; i++)
	{
	  if (i > 0)
	    os << ", ";
	  output_nicely (os, xform (i, j));
	}
    }

  os << ">";

  return os;
}
