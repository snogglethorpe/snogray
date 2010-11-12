// vec-io.h -- Debugging output for Vec type
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

#ifndef __VEC_IO_H__
#define __VEC_IO_H__

#include <iosfwd>

#include "vec.h"

namespace snogray {

std::ostream& operator<< (std::ostream &os, const Vec &vec);

}

#endif // __VEC_IO_H__
