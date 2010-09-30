// thread.h -- thread wrapper
//
//  Copyright (C) 2009, 2010  Miles Bader <miles@gnu.org>
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


#ifndef __THREAD_H__
#define __THREAD_H__

#include "config.h"

#if USE_STD_THREAD
#include <thread>
#elif USE_BOOST_THREAD
#include <boost/thread/thread.hpp>
#endif


namespace snogray {


#if USE_STD_THREAD

typedef std::thread RealThread;

#elif USE_BOOST_THREAD

typedef boost::thread RealThread;

#endif // !USE_STD_THREAD && !USE_BOOST_THREAD


// Thread is a thin wrapper that just inherits a selected set of
// operations from RealThread.  The main intent of the wrapper is to
// define a minimal subset of C++0x std::thread, exporting only those
// few operations we use to avoid inadvertent dependencies on
// particular thread implementations.
//
class Thread : public RealThread
{
public:

  // Create a thread, which will call the OBJ->*METH method and then
  // exit.
  //
  template<class C1, class C2>
  Thread (void (C1::*meth)(), C2 *obj) : RealThread (meth, obj) { }

  // Create a thread, which will call FUNCTOR and then exit.  FUNCTOR
  // is copied unless something like std::ref is used.
  //
  template<typename F>
  Thread (const F &functor) : RealThread (functor) { }

  // Create a thread, which will call FUNCTOR with an argument of ARG
  // and then exit.  FUNCTOR and ARG will be copied unless something
  // like std::ref is used.
  //
  template<typename F, typename A>
  Thread (const F &functor, const A &arg) : RealThread (functor, arg) { }

  using RealThread::join;
};


}


#endif // __THREAD_H__
