// render-queue.h -- thread-safe queue of RenderPackets
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RENDER_QUEUE_H
#define SNOGRAY_RENDER_QUEUE_H

#include <deque>

#include "util/mutex.h"
#include "util/cond-var.h"


namespace snogray {


class RenderPacket;


// A thread-safe queue of RenderPackets, used to pass packets between
// threads.
//
class RenderQueue
{
public:

  RenderQueue () : done (false) { }

  // Return the next packet from this queue, removing it from the queue.
  // If the queue is empty, will first wait for a packet to be added,
  // unless RenderQueue::shutdown() has been called, in which case it will
  // return a null pointer instead.
  //
  RenderPacket *get ();

  // Return a packet from this queue, without any locking (so should only
  // be called in contexts where no conflict with other threads is
  // possible),  or a null pointer if the queue is empty.
  //
  RenderPacket *get_unlocked ()
  {
    if (packets.empty ())
      return 0;
	
    RenderPacket *packet = packets.front ();
    packets.pop_front ();
    return packet;
  }

  // Add a packet to this queue, possibly waking up a thread which is
  // waiting for one.
  //
  void put (RenderPacket *packet);

  // Cause RenderQueue::get() to return a null pointer when the queue is
  // empty, instead of waiting.
  //
  void shutdown ();

  bool empty () const { return packets.empty (); }

private:

  std::deque<RenderPacket *> packets;

  // Mutex used to protect RenderQueue::packets.
  //
  Mutex mutex;

  // Condition variable used for awaiting the arrival of new packets in
  // RenderQueue::packets.
  //
  CondVar cond;

  // If true, then RenderQueue::get will return a null pointer for an empty
  // queue, instead of waiting.
  //
  bool done;
};


}

#endif // SNOGRAY_RENDER_QUEUE_H
