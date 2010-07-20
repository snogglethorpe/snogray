// render-mgr.h -- Outer rendering driver
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

#ifndef __RENDER_MGR_H__
#define __RENDER_MGR_H__

#include "image-output.h"
#include "render-pattern.h"


namespace snogray {


class Camera;
class Progress;
class RenderPacket;
class RenderStats;
class GlobalRenderState;


class RenderMgr
{
public:

  // The number of results (roughly) we try to put in each packet.
  //
  static const unsigned PACKET_SIZE = 4096;

  RenderMgr (const GlobalRenderState &global_state,
	     const Camera &_camera, unsigned _width, unsigned _height);

  // Render the pixels in PATTERN to OUTPUT.  PROG will be periodically
  // updated using the value of RenderPattern::position on an iterator
  // iterating through PATTERN.  STATS will be updated with rendering
  // statistics.
  //
  void render (RenderPattern &pattern, ImageOutput &output,
	       Progress &prog, RenderStats &stats);

private:

  // Render the pixels in PATTERN to OUTPUT, using only the current
  // thread.  PROG will be periodically updated using the value of
  // RenderPattern::position on an iterator iterating through PATTERN.
  // STATS will be updated with rendering statistics.
  //
  void render_single_threaded (RenderPattern &pattern, ImageOutput &output,
			       Progress &prog, RenderStats &stats);

  // Fill PACKET with pixels yielded from PAT_IT.
  //
  void fill_packet (RenderPattern::iterator &pat_it,
		    const RenderPattern::iterator &limit,
		    RenderPacket &packet);

  // Output results from PACKET to OUTPUT.
  //
  void output_packet (RenderPacket &packet, ImageOutput &output);

  const GlobalRenderState &global_state;

  // The camera being used.
  //
  const Camera &camera;

  // Size of the virtual screen being rendered to, which has pixel
  // coordinates (0 - width-1, 0 - height-1).  These are floats because
  // they are always used as such.
  //
  float width, height;
};


}

#endif // __RENDER_MGR_H__
