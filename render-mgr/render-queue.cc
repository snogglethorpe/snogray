// render-queue.cc -- thread-safe queue of RenderPackets
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "render-queue.h"

using namespace snogray;


// Return the next packet from this queue, removing it from the queue.
// If the queue is empty, will first wait for a packet to be added,
// unless RenderQueue::shutdown() has been called, in which case it will
// return a null pointer instead.
//
RenderPacket *
RenderQueue::get ()
{
  UniqueLock lock (mutex);

  while (packets.empty () && !done)
    cond.wait (lock);

  return get_unlocked ();
}

// Add a packet to this queue, possibly waking up a thread which is
// waiting for one.
//
void
RenderQueue::put (RenderPacket *packet)
{
  UniqueLock lock (mutex);
  packets.push_back (packet);
  cond.notify_one ();
}

// Cause RenderQueue::get() to return a null pointer when the queue is
// empty, instead of waiting.
//
void
RenderQueue::shutdown ()
{
  if (! done)
    {
      LockGuard lock (mutex);
      done = true;
      cond.notify_all ();
    }
}
