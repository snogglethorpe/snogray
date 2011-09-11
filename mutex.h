// mutex.h -- mutex wrapper
//
//  Copyright (C) 2009, 2010, 2011  Miles Bader <miles@gnu.org>
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


#ifndef SNOGRAY_MUTEX_H
#define SNOGRAY_MUTEX_H

#include "config.h"

#if USE_STD_THREAD
#include <mutex>
#elif USE_BOOST_THREAD
#include <boost/thread/mutex.hpp>
#endif


namespace snogray {


#if USE_STD_THREAD

typedef std::mutex RealMutex;
typedef std::unique_lock<std::mutex> RealUniqueLock;

#elif USE_BOOST_THREAD

typedef boost::mutex RealMutex;
typedef boost::unique_lock<boost::mutex> RealUniqueLock;

#else // !USE_STD_THREAD && !USE_BOOST_THREAD

//
// Empty definitions of RealMutex and RealUniqueLock classes, for use
// when multi-threading is not availble.
//

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

#endif // !USE_STD_THREAD && !USE_BOOST_THREAD


// Mutex is a thin wrapper that just inherits a selected set of
// operations from RealMutex.  The main intent of the wrapper is to
// export only those few operations we use, to avoid inadvertent
// dependencies on particular mutex implementations.
//
class Mutex : RealMutex
{
public:

  using RealMutex::lock;
  using RealMutex::unlock;

  // Return the underlying mutex type.
  //
  RealMutex &real_mutex () { return *this; }
};

// A LockGuard holds a mutex locked for the duration of its existance.
//
// This is a standard class in c++0x std threads and boost threads, but
// it's so simple that it's easier just to reimplement it ourselves.
//
// Note that unlike std::lock_guard, LockGuard is not a template, and
// only works with Mutex (whereas std::lock_guard works with arbitrary
// mutex types).
//
class LockGuard
{
public:

  LockGuard (Mutex &_m) : m (_m) { m.lock (); }
  ~LockGuard () { m.unlock (); }

private:

  Mutex &m;
};

// A UniqueLock is like LockGuard, but more powerful and more complex;
// it is used by the CondVar (condition variable) class.
//
// UniqueLock is a thin wrapper that just inherits a selected set of
// operations from RealUniqueLock.  The main intent of the wrapper is to
// export only those few operations we use, to avoid inadvertent
// dependencies on particular implementations.
//
// Note that unlike std::unique_lock, UniqueLock is not a template, and
// only works with Mutex (whereas std::unique_lock works with arbitrary
// mutex types).
//
class UniqueLock : RealUniqueLock
{
public:

  UniqueLock () { }

  explicit UniqueLock (Mutex &mutex)
    : RealUniqueLock (mutex.real_mutex ())
  { }

  template<typename A>
  UniqueLock (Mutex &mutex, const A &arg)
    : RealUniqueLock (mutex.real_mutex (), arg)
  { }

  // Return the underlying type.
  //
  RealUniqueLock &real_unique_lock () { return *this; }
};


}


#endif // SNOGRAY_MUTEX_H
