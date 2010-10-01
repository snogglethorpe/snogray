// render-thread.cc -- single rendering thread
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

#include "render-thread.h"


using namespace snogray;


void
RenderWorker::run ()
{
  while (RenderPacket *packet = in_q.get ())
    {
      renderer.render_packet (*packet);
      out_q.put (packet);
    }
}
