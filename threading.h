// threading.h -- Low-level threading
//
//  Copyright (C) 2009-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// If threading is enabled, Thread is a wrapper for an appropriate system
// thread object.  Otherwise, it provides empty definitions for various
// thread methods (so it should be always usable, even on systems without
// threading support).
//

#ifndef SNOGRAY_THREADING_H
#define SNOGRAY_THREADING_H

#include "config.h"


// Based on our configuration, include an implementation-specific
// header file to define the classes "RealThread", "RealMutex",
// "RealLockGuard", and "RealCondVar".
//
#if USE_STD_THREAD

#include "threading-std.h"

#elif USE_BOOST_THREAD

#include "threading-boost.h"

#else
// Provide empty definitions of the low-level threading-related
// classes, for use when multi-threading is not availble.  Note that
// we _don't_ provide an empty definition of RealThread, as it can
// never really be a nop; places that actually create threads need to
// handle the no-threading case explicitly.

class RealMutex
{
public:

  void lock () { }
  void unlock () { }
};

class RealUniqueLock
{
public:

  RealUniqueLock () { }
  explicit RealUniqueLock (RealMutex &) { }
  template<typename A> RealUniqueLock (RealMutex &, const A &) { }
};

class RealCondVar
{
public:

  void notify_one() { }
  void notify_all() { }
  template <typename L> void wait (L &) { }
};

#endif // !USE_STD_THREAD && !USE_BOOST_THREAD


#endif // SNOGRAY_THREADING_H
