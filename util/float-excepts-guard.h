// float-excepts-guard.h -- enable/disable floating-point exceptions
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

#ifndef SNOGRAY_FLOAT_EXCEPTS_GUARD_H
#define SNOGRAY_FLOAT_EXCEPTS_GUARD_H

#include "config.h"

#if USE_FP_EXCEPTIONS && HAVE_FENV_H
#include <fenv.h>
#endif


namespace snogray {


// This object acts as a "guard", enabling and disabling a given set
// floating-point exceptions in the constructor, and reversing the
// action in the destructor (restoring the previous default
// condition).
//
// It is a wrapper for feenableexcept, and so uses the same flag
// values.  In an environment where feenableexcept isn't supported, it
// has no effect.
//
// Note that the effects of this are _global_, so it must be used with
// care.
//
class FloatExceptsGuard
{
public:

  FloatExceptsGuard (int enable, int disable = 0)
    : previous_mask (0), new_mask (0)
  {
#if USE_FP_EXCEPTIONS && HAVE_FEENABLEEXCEPT
    if (enable)
      {
	previous_mask = feenableexcept (enable);
	new_mask = previous_mask | enable;
      }
    if (disable)
      {
	previous_mask = fedisableexcept (disable);
	new_mask = previous_mask & ~disable;
      }
#endif // USE_FP_EXCEPTIONS && HAVE_FEENABLEEXCEPT
  }

  ~FloatExceptsGuard ()
  {
#if USE_FP_EXCEPTIONS && HAVE_FEENABLEEXCEPT
    int re_enable = previous_mask & ~new_mask;
    int re_disable = new_mask & ~previous_mask;
    if (re_enable)
      feenableexcept (re_enable);
    if (re_disable)
      fedisableexcept (re_disable);
#endif // USE_FP_EXCEPTIONS && HAVE_FEENABLEEXCEPT
  }

private:

  int previous_mask, new_mask;
};


// Define any unsupported FE_ exception flags to be zero, which lets
// us users just use them directly without #ifdefs.
//
#ifndef FE_INEXACT
# define FE_INEXACT 0
#endif
#ifndef FE_DIVBYZERO
# define FE_DIVBYZERO 0
#endif
#ifndef FE_UNDERFLOW
# define FE_UNDERFLOW 0
#endif
#ifndef FE_OVERFLOW
# define FE_OVERFLOW 0
#endif
#ifndef FE_INVALID
# define FE_INVALID 0
#endif
#ifndef FE_ALL_EXCEPT
# define FE_ALL_EXCEPT 0
#endif


}

#endif // SNOGRAY_FLOAT_EXCEPTS_GUARD_H
