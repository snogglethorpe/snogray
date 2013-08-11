// threading-std.h -- Threading support using C++11 std:: types
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

#ifndef SNOGRAY_THREADING_STD_H
#define SNOGRAY_THREADING_STD_H

#include <thread>
#include <mutex>
#include <condition_variable>


namespace snogray {

typedef std::thread RealThread;

typedef std::mutex RealMutex;
typedef std::unique_lock<std::mutex> RealUniqueLock;

typedef std::condition_variable RealCondVar;

}


#endif // SNOGRAY_THREADING_STD_H
