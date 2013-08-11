// render-thread.h -- single rendering thread
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

#ifndef SNOGRAY_RENDER_THREAD_H
#define SNOGRAY_RENDER_THREAD_H

#include "util/thread.h"

#include "renderer.h"


namespace snogray {


class RenderQueue;
class GlobalRenderState;
class Camera;


// The guts of a single rendering thread.
//
class RenderWorker
{
public:

  RenderWorker (const GlobalRenderState &global_state,
		const Camera &camera, unsigned width, unsigned height,
		RenderQueue &_in_q, RenderQueue &_out_q)
    : renderer (global_state, camera, width, height),
      in_q (_in_q), out_q (_out_q)
  { }

  // Return rendering statistics from this thread.
  //
  RenderStats stats () const { return renderer.stats (); }

  void run ();

private:

  // Per-thread rendering state.
  //
  Renderer renderer;

  // RenderPacket queues for communicating with the global thread manager.
  // IN_Q holds packets to be rendered, and OUT_Q holds packets with
  // rendering results.
  //
  RenderQueue &in_q, &out_q;
};

// Thread that runs a RenderWorker.
//
class RenderThread : public RenderWorker, public Thread
{
public:

  RenderThread (const GlobalRenderState &global_state,
		const Camera &camera, unsigned width, unsigned height,
		RenderQueue &_in_q, RenderQueue &_out_q)
    : RenderWorker (global_state, camera, width, height, _in_q, _out_q),
      Thread (&RenderThread::run, this)
  { }
};


}

#endif // SNOGRAY_RENDER_THREAD_H
