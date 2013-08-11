// nice-io.h -- Helper functions for user-friendly I/O
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_NICE_IO_H
#define SNOGRAY_NICE_IO_H

#include <iosfwd>

namespace snogray {

// Prints NUM in a "nice" format, in a way that looks good and
// is easy to read, even if it's not that accurate (e.g., the
// precision is limited to 3-4 digits, and small numbers are just
// treated as 0).  This is used for debugging output, etc.
//
void
output_nicely (std::ostream &os, float num);

}

#endif // SNOGRAY_NICE_IO_H
