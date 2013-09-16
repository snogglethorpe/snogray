// renderer.cc -- Output rendering object
//
//  Copyright (C) 2006-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <list>
#include <map>

#include "util/snogmath.h"
#include "util/snogassert.h"
#include "util/progress.h"
#include "util/float-excepts-guard.h"
#include "render/global-render-state.h"
#include "renderer.h"
#include "render-packet.h"
#if USE_THREADS
#include "render-thread.h"
#include "render-queue.h"
#endif

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
RenderMgr::render (unsigned num_threads,
		   RenderPattern &pattern,
		   ImageSampledOutput &output,
		   Progress &prog, RenderStats &stats)
{
  // Tell the progress indicator the bounds we will be using.
  //
  prog.set_start (pattern.position (pattern.begin ()));
  prog.set_size (pattern.position (pattern.end ())
		 - pattern.position (pattern.begin ()));

  // Turn on floating-point exceptions during rendering if possible,
  // to detect errors.
  //
  FloatExceptsGuard fe_guard (FE_DIVBYZERO|FE_INVALID);

  // Render!
  //
#if USE_THREADS
  if (num_threads != 1)
    render_multi_threaded (num_threads, pattern, output, prog, stats);
  else
#endif // USE_THREADS
    render_single_threaded (pattern, output, prog, stats);
}


// single-threaded rendering

// Render the pixels in PATTERN to OUTPUT, using only the current
// thread.  PROG will be periodically updated using the value of
// RenderPattern::position on an iterator iterating through PATTERN.
// STATS will be updated with rendering statistics.
//
void
RenderMgr::render_single_threaded (RenderPattern &pattern,
				   ImageSampledOutput &output,
				   Progress &prog, RenderStats &stats)
{
  Renderer renderer (global_state, camera, width, height);
  RenderPattern::iterator pat_it = pattern.begin ();
  RenderPattern::iterator limit = pattern.end ();
  RenderPacket packet;

  prog.start ();

  while (pat_it != limit)
    {
      output.set_min_sample_y (
	       clamp (pattern.min_y (pat_it), 0, int (height) - 1));

      fill_packet (pat_it, limit, packet);

      renderer.render_packet (packet);

      output_packet (packet, output);

      prog.update (pattern.position (pat_it));
    }

  prog.end ();

  stats += renderer.stats ();
}


// multi-threaded rendering

#if USE_THREADS

// Render the pixels in PATTERN to OUTPUT, using NUM_THREADS threads.
// PROG will be periodically updated using the value of
// RenderPattern::position on an iterator iterating through PATTERN.
// STATS will be updated with rendering statistics.
//
void
RenderMgr::render_multi_threaded (unsigned num_threads,
				  RenderPattern &pattern,
				  ImageSampledOutput &output,
				  Progress &prog, RenderStats &stats)
{
  RenderPattern::iterator pat_it = pattern.begin ();
  RenderPattern::iterator limit = pattern.end ();

  // RenderPacket queues for communicating with rendering threads.
  // PENDING_Q holds packets with pixels to be rendered, and DONE_Q holds
  // packets with the results.
  //
  RenderQueue pending_q, done_q;

  unsigned num_packets = num_threads * 2;

  // A mapping from packets to "min_y" values.
  //
  std::map<RenderPacket *, int> packet_min_y;

  // To start, we just add new empty packets to DONE_Q.  They'll be
  // processed as if they contain results, but with no effect because
  // they're empty, and then fed back into the processing loop.
  //
  for (unsigned i = 0; i < num_packets; i++)
    done_q.put (new RenderPacket);

  // Now start our rendering threads; they'll just block waiting for
  // packets to be added to PENDING_Q.
  //
  std::list<RenderThread *> threads;
  for (unsigned i = 0; i < num_threads; i++)
    threads.push_back (new RenderThread (global_state, camera, width, height,
					 pending_q, done_q));

  prog.start ();

  while (pat_it != limit)
    {
      RenderPacket *packet = done_q.get ();
      ASSERT (packet);

      // Write out the results from PACKET.
      //
      output_packet (*packet, output);

      // Update PACKET's min_y value to reflect the pixels it will be
      // filled with.
      //
      packet_min_y[packet]
	= clamp (pattern.min_y (pat_it), 0, int (height) - 1);

      // Compute the "global min_y" value, which is the minimum of all
      // packet min_y values.
      //
      int global_min_y = int (height) - 1;
      for (std::map<RenderPacket *, int>::const_iterator i
	     = packet_min_y.begin();
	   i != packet_min_y.end (); ++i)
	global_min_y = min (global_min_y, i->second);

      // Set OUTPUT's min_y accordingly.
      //
      output.set_min_sample_y (global_min_y);

      // Now add more pixels to PACKET and make it available for more
      // processing.
      //
      fill_packet (pat_it, limit, *packet);
      pending_q.put (packet);

      prog.update (pattern.position (pat_it));
    }

  // Call RenderQueue::shutdown on PENDING_Q and DONE_Q so that that calls
  // to RenderQueue::get won't block when the queues finally run out, but
  // instead will return a null pointer.  This allows orderly shutdown.
  //
  pending_q.shutdown ();
  done_q.shutdown ();

  // Join and destroy all rendering threads, which should have finished by
  // now.
  //
  while (! threads.empty ())
    {
      RenderThread *th = threads.back ();
      threads.pop_back ();
      th->join ();
      stats += th->stats ();
      delete th;
    }

  // Get the final batch of results, and destroy the packets.
  //
  while (RenderPacket *packet = done_q.get ())
    {
      output_packet (*packet, output);
      delete packet;
    }

  ASSERT (pending_q.empty ());
  ASSERT (done_q.empty ());

  prog.end ();
}

#endif // USE_THREADS


// packet utility methods

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
RenderMgr::output_packet (RenderPacket &packet, ImageSampledOutput &output)
{
  for (std::vector<RenderPacket::Result>::iterator ri = packet.results.begin ();
       ri != packet.results.end (); ++ri)
    output.add_sample (ri->coords.u, ri->coords.v, ri->val);
}
