// timeval.cc -- Time measurement
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <sstream>
#include <iomanip>

#include "timeval.h"

using namespace Snogray;
using namespace std;

string
Timeval::fmt () const
{
  unsigned sec  = tv_sec % 60;
  unsigned min  = (tv_sec / 60) % 60;
  unsigned hr   = tv_sec / 3600;
  unsigned msec = (tv_usec + 500) / 1000;

#if 0 /* ostringstream crashes, so fuck it */
  ostringstream ss;

  ss.fill ('0');

  if (hr > 0)
    ss << hr << ':' << setw (2) << min << ':' << setw (2) << sec;
  else if (min > 0)
    ss << min << ':' << setw (2) << sec;
  else
    ss << sec;

  ss << '.' << setw (3) << msec;

  return ss.str ();
#else
  char buf[20];

  if (hr > 0)
    snprintf (buf, sizeof buf, "%u:%02u:%02u.%03u", hr, min, sec, msec);
  else if (min > 0)
    snprintf (buf, sizeof buf, "%u:%02u.%03u", min, sec, msec);
  else
    snprintf (buf, sizeof buf, "%u.%03u", sec, msec);

  return string (buf);
#endif
}

// arch-tag: 650b9486-0140-4dba-be6e-639589dbc7df
