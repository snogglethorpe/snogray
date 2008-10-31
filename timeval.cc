// timeval.cc -- Time measurement
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <sstream>
#include <iomanip>
#include <cstdio>

#include "timeval.h"

using namespace snogray;
using namespace std;

string
Timeval::fmt (unsigned sub_sec_prec) const
{
  unsigned sec  = tv_sec % 60;
  unsigned min  = (tv_sec / 60) % 60;
  unsigned hr   = tv_sec / 3600;

  if (sub_sec_prec == 0 && tv_usec > 500000)
    {
      sec++;
      if (sec == 60)
	{
	  min++;
	  if (min == 60)
	    hr++;
	}
    }

  // (ostringstream crashes so we just use snprintf)

  char buf[20], *end = buf;

  if (hr > 0)
    end += snprintf (buf, sizeof buf, "%u:%02u:%02u", hr, min, sec);
  else if (min > 0)
    end += snprintf (buf, sizeof buf, "%u:%02u", min, sec);
  else 
    end += snprintf (buf, sizeof buf, "%u", sec);

  if (sub_sec_prec > 0)
    {
      unsigned div = 1;

      for (unsigned i = 6; i > sub_sec_prec; i--)
	div *= 10;

      unsigned frac = (tv_usec + (div >> 1)) / div;

      end += snprintf(end, buf + sizeof buf - end, ".%0*u", sub_sec_prec, frac);
    }

  if (hr == 0 && min == 0 && end < buf + sizeof buf - 1)
    {
      *end++ = 's';
      *end = '\0';
    }

  return string (buf);
}

// arch-tag: 650b9486-0140-4dba-be6e-639589dbc7df
