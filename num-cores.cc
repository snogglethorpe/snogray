// num-cores-var.cc -- return the number of cores on this system
//
//  Copyright (C) 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <unistd.h> // sysconf

#include "num-cores.h"


using namespace snogray;


// If the number of available CPU cores on this system can be
// determined, return it, otherwise return DEFAULT_CORES.
//
int
snogray::num_cores (int default_cores)
{
#if USE_THREADS
#ifdef _SC_NPROCESSORS_ONLN
  // This works on linux, anyway.
  int sc = sysconf (_SC_NPROCESSORS_ONLN);
  if (sc > 0)
    return sc;
#endif // _SC_NPROCESSORS_ONLN
#endif // USE_THREADS

  return default_cores;
}
