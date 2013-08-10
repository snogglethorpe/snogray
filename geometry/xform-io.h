// xform-io.h -- Debugging output for Xform type
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

#ifndef SNOGRAY_XFORM_IO_H
#define SNOGRAY_XFORM_IO_H

#include <iosfwd>

#include "xform.h"

namespace snogray {

std::ostream& operator<< (std::ostream &os, const Xform &xform);

}

#endif // SNOGRAY_XFORM_IO_H
