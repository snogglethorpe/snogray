// rusage.h -- Resource usage measurement
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RUSAGE_H__
#define __RUSAGE_H__

#include <sys/resource.h>

#include "timeval.h"

namespace Snogray {

struct Rusage : rusage
{
  Rusage () { getrusage (RUSAGE_SELF, static_cast<struct rusage *>(this)); }

  Timeval utime () const { return Timeval (ru_utime); }
  Timeval stime () const { return Timeval (ru_stime); }
};

}

#endif /* __RUSAGE_H__ */

// arch-tag: 7a4422b7-3908-44f2-9105-95b6905b6905
