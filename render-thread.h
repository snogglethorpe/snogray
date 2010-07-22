// render-thread.h -- single rendering thread
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

#ifndef __RENDER_THREAD_H__
#define __RENDER_THREAD_H__

#include "thread.h"

#include "renderer.h"


namespace snogray {


class RenderQueue;
class GlobalRenderState;
class Camera;


//
class RenderThread
{
public:

  RenderThread (const GlobalRenderState &global_state,
		const Camera &camera, unsigned width, unsigned height,
		RenderQueue &_in_q, RenderQueue &_out_q)
    : renderer (global_state, camera, width, height),
      in_q (_in_q), out_q (_out_q), thread (invoke, this)
  { }

  // Wait until the underlying thread finishes.
  //
  void join () { thread.join (); }

  // Return rendering statistics from this thread.
  //
  RenderStats stats () const { return renderer.stats (); }

private:

  static void invoke (RenderThread *x) { x->run (); }

  void run ();

  // Per-thread rendering state.
  //
  Renderer renderer;

  // RenderPacket queues for communicating with the global thread manager.
  // IN_Q holds packets to be rendered, and OUT_Q holds packets with
  // rendering results.
  //
  RenderQueue &in_q, &out_q;

  // Must be the last field in this class, so that all other fields are
  // initialized when the actual thread starts running.
  //
  Thread thread;
};


}

#endif // __RENDER_THREAD_H__
