// cond-var.h -- condition_variable wrapper
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// If threading is enabled, CondVar is a wrapper for an appropriate system
// conditional_variable object.  Otherwise, it provides empty definitions
// for various conditional_variable methods (so it should be always usable,
// even on systems without threading support).
//


#ifndef SNOGRAY_COND_VAR_H
#define SNOGRAY_COND_VAR_H

#include "config.h"

#if USE_STD_THREAD
#include <condition_variable>
#elif USE_BOOST_THREAD
#include <boost/thread/condition_variable.hpp>
#endif

#include "mutex.h"


namespace snogray {


#if USE_STD_THREAD

typedef std::condition_variable RealCondVar;

#elif USE_BOOST_THREAD

typedef boost::condition_variable RealCondVar;

#else // !USE_STD_THREAD && !USE_BOOST_THREAD

// A nop condition_variable class.
//
class RealCondVar
{
public:

  void notify_one() { }
  void notify_all() { }
  template <typename L> void wait (L &) { }
};

#endif // !USE_STD_THREAD && !USE_BOOST_THREAD


// CondVar is a thin wrapper that just inherits a selected set of
// operations from RealCondVar.  The main intent of the wrapper is to
// export only those few operations we use, to avoid inadvertent
// dependencies on particular mutex implementations.
//
class CondVar : RealCondVar
{
public:

  using RealCondVar::notify_one;
  using RealCondVar::notify_all;

  void wait (UniqueLock &lock)
  {
    RealCondVar::wait (lock.real_unique_lock ());
  }
};


}


#endif // SNOGRAY_COND_VAR_H
