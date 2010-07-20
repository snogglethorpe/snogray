// renderer.cc -- Output rendering object
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "progress.h"
#include "renderer.h"
#include "render-packet.h"

#include "render-mgr.h"


using namespace snogray;


RenderMgr::RenderMgr (const GlobalRenderState &_global_state,
		      const Camera &_camera,
		      unsigned _width, unsigned _height)
  : global_state (_global_state), camera (_camera),
    width (_width), height (_height)
{
}



// Render the pixels in PATTERN to OUTPUT.  PROG will be periodically
// updated using the value of RenderPattern::position on an iterator
// iterating through PATTERN.  STATS will be updated with rendering
// statistics.
//
void
RenderMgr::render (RenderPattern &pattern, ImageOutput &output,
		   Progress &prog, RenderStats &stats)
{
  // For now, just use one thread.
  //
  render_single_threaded (pattern, output, prog, stats);
}


// Render the pixels in PATTERN to OUTPUT, using only the current
// thread.  PROG will be periodically updated using the value of
// RenderPattern::position on an iterator iterating through PATTERN.
// STATS will be updated with rendering statistics.
//
void
RenderMgr::render_single_threaded (RenderPattern &pattern, ImageOutput &output,
				   Progress &prog, RenderStats &stats)
{
  Renderer renderer (global_state, camera, width, height);
  RenderPattern::iterator pat_it = pattern.begin ();
  RenderPattern::iterator limit = pattern.end ();
  RenderPacket packet;

  prog.start ();

  while (pat_it != limit)
    {
      output.set_min_y (clamp (pattern.min_y (pat_it), 0, int (height) - 1));

      fill_packet (pat_it, limit, packet);

      renderer.render_packet (packet);

      output_packet (packet, output);

      prog.update (pattern.position (pat_it));
    }

  prog.end ();

  stats = renderer.stats ();
}

// Fill PACKET with pixels yielded from PAT_IT.
//
void
RenderMgr::fill_packet (RenderPattern::iterator &pat_it,
			const RenderPattern::iterator &limit,
			RenderPacket &packet)
{
  packet.pixels.clear ();
  packet.results.clear ();

  // Calculate the number of input pixels which will yield the desired
  // number of output results.
  //
  unsigned num_samps = global_state.num_samples;
  unsigned num_pix = (PACKET_SIZE + num_samps - 1) / num_samps;

  for (unsigned i = 0; i < num_pix && pat_it != limit; i++)
    packet.pixels.push_back (*pat_it++);
}

// Output results from PACKET to OUTPUT.
//
void
RenderMgr::output_packet (RenderPacket &packet, ImageOutput &output)
{
  for (std::vector<RenderPacket::Result>::iterator ri = packet.results.begin ();
       ri != packet.results.end (); ++ri)
    output.add_sample (ri->coords.u, ri->coords.v, ri->val);
}
