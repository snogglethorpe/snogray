// timeval.h -- Time measurement
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TIMEVAL_H__
#define __TIMEVAL_H__

#include <string>

#include <sys/time.h>

namespace Snogray {

struct Timeval : timeval
{
  typedef enum { TIME_OF_DAY } __tod_t;

  // Type trick; only gets used when arg is Timeval::TIME_OF_DAY
  Timeval (__tod_t) { gettimeofday (static_cast<timeval *>(this), 0); }

  Timeval (const timeval &tv) : timeval (tv) { }

  template<typename T>
  Timeval (T sec, unsigned long usec = 0)
  {
    tv_sec = time_t (sec);
    if (T (tv_sec) != sec)
      {
	sec -= tv_sec;
	usec += static_cast<unsigned long> (sec * 1000000);
      }	
    tv_usec = usec;
  }

  operator double () const { return tv_sec + double (tv_usec) / 1000000.0; }

  Timeval operator- (const Timeval &tv2) const
  {
    time_t sec = tv_sec - tv2.tv_sec;
    long usec = tv_usec - tv2.tv_usec;

    if (usec < 0)
      {
	sec -= 1;
	usec += 1000000;
      }

    return Timeval (sec, usec);
  }
  Timeval operator+ (const Timeval &tv2) const
  {
    time_t sec = tv_sec + tv2.tv_sec;
    long usec = tv_usec + tv2.tv_usec;

    if (usec > 1000000)
      {
	sec += 1;
	usec -= 1000000;
      }

    return Timeval (sec, usec);
  }

  std::string fmt (unsigned sub_sec_prec = 0) const;
};

static inline std::ostream&
operator<< (std::ostream &os, const Timeval &tv)
{
  os << tv.fmt ();
  return os;
}

}

#endif /* __TIMEVAL_H__ */

// arch-tag: 22923e7e-e21c-4faf-88a3-13bb51149478
