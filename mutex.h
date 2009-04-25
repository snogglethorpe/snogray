// mutex.h -- mutex wrapper
//
//  Copyright (C) 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// If threading is enabled, Mutex is a wrapper for an appropriate system
// mutex object.  Otherwise, it provides empty definitions for various
// mutex methods (so it should be always usable, even on systems without
// threading support).
//


#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "config.h"

#if USE_STD_THREAD
#include <mutex>
#elif USE_BOOST_THREAD
#include <boost/thread/mutex.hpp>
#endif


namespace snogray {


#if USE_STD_THREAD

typedef std::mutex RealMutex;

#elif USE_BOOST_THREAD

typedef boost::mutex RealMutex;

#else // !USE_STD_THREAD && !USE_BOOST_THREAD

// A nop mutex class.
//
class RealMutex
{
public:

  void lock () { }
  void unlock () { }
};

#endif // !USE_STD_THREAD && !USE_BOOST_THREAD


// Mutex is a thin wrapper that just inherits a selected set of
// operations from RealMutex.
//
class Mutex : RealMutex
{
public:

  using RealMutex::lock;
  using RealMutex::unlock;
};


}


#endif // __MUTEX_H__
