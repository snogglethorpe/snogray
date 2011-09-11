// rusage.h -- Resource usage measurement
//
//  Copyright (C) 2005, 2007, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RUSAGE_H
#define SNOGRAY_RUSAGE_H

#include <sys/resource.h>

#include "timeval.h"

namespace snogray {

struct Rusage : rusage
{
  Rusage () { getrusage (RUSAGE_SELF, static_cast<struct rusage *>(this)); }

  Timeval utime () const { return Timeval (ru_utime); }
  Timeval stime () const { return Timeval (ru_stime); }
};

}

#endif /* SNOGRAY_RUSAGE_H */

// arch-tag: 7a4422b7-3908-44f2-9105-95b6905b6905
