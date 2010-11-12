// nice-io.cc -- Helper functions for user-friendly I/O
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

#include "snogmath.h"

#include "nice-io.h"

// Prints NUM in a "nice" format, in a way that looks good and
// is easy to read, even if it's not that accurate (e.g., the
// precision is limited to 3-4 digits, and small numbers are just
// treated as 0).  This is used for debugging output, etc.
//
void
snogray::output_nicely (std::ostream &os, float num)
{
  if (abs (num) < 1.e-7)
    os << "0";
  else
    {
      unsigned oprec = os.precision (4);
      std::ios::fmtflags oflags
	= os.setf (std::ios::fmtflags(0), std::ios::floatfield);

      os << num;

      os.precision (oprec);
      os.flags (oflags);
    }
}
