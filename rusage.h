// rusage.h -- Resource usage measurement
//
//  Copyright (C) 2005, 2007, 2011, 2012  Miles Bader <miles@gnu.org>
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


namespace snogray {


struct Rusage : rusage
{
  Rusage () { getrusage (RUSAGE_SELF, static_cast<struct rusage *>(this)); }

  double utime () const
  {
    return double (ru_utime.tv_sec) + double (ru_utime.tv_usec) / 1e6;
  }
  double stime () const
  {
    return double (ru_stime.tv_sec) + double (ru_stime.tv_usec) / 1e6;
  }
};


}

#endif // SNOGRAY_RUSAGE_H
