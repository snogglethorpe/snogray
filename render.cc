// render.cc -- Main rendering loop
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <memory>

#include "renderer.h"
#include "progress.h"
#include "grid.h"
#include "sample2-gen.h"

#include "render.h"

using namespace Snogray;


static void
render_by_rows (const Scene &scene, const Camera &camera,
		unsigned width, unsigned height,
		ImageOutput &output, unsigned offs_x, unsigned offs_y,
		Sample2Gen &sample_gen,
		const TraceParams &trace_params, TraceStats &stats,
		std::ostream &prog_stream, Progress::Verbosity verbosity)
{
  // Start progress indicator
  //
  Progress prog (prog_stream, "line", offs_y, offs_y + output.height,
		 verbosity);

  prog.start ();

  Renderer renderer (scene, camera, width, height, output, offs_x, offs_y,
		     1, sample_gen, trace_params);

  for (unsigned row_offs = 0; row_offs < output.height; row_offs++)
    {
      renderer.render_block (offs_x, offs_y + row_offs, output.width, 1);
      prog.update (offs_y + row_offs);
    }

  stats = renderer.trace_stats ();

  prog.end ();
}



static void
render_by_blocks (unsigned block_width, unsigned block_height,
		  const Scene &scene, const Camera &camera,
		  unsigned width, unsigned height,
		  ImageOutput &output, unsigned offs_x, unsigned offs_y,
		  Sample2Gen &sample_gen,
		  const TraceParams &trace_params, TraceStats &stats,
		  std::ostream &prog_stream, Progress::Verbosity verbosity)
{
  unsigned num_block_rows = (output.height + block_height - 1) / block_height;
  unsigned num_block_cols = (output.width + block_width - 1) / block_width;
  unsigned num_blocks = num_block_cols * num_block_rows;

  // Start progress indicator
  //
  Progress prog (prog_stream, "block", 0, num_blocks, verbosity);

  prog.start ();

  Renderer renderer (scene, camera, width, height, output, offs_x, offs_y,
		     block_height, sample_gen, trace_params);

  unsigned cur_block_num = 0;

  // Iterate over every block, rendering each one.  Note that in the
  // case of a recovered image, many of these blocks may be outside
  // OUTPUT's "min_y" limit; that's OK -- rendering them will simply
  // have no effect.
  //
  for (unsigned block_y_offs = 0;
       block_y_offs < output.height;
       block_y_offs += block_height)
    {
      for (unsigned block_x_offs = 0;
	   block_x_offs < output.width;
	   block_x_offs += block_width)
	{
	  unsigned cur_block_width
	    = ((block_x_offs + block_width > output.width)
	       ? output.width - block_x_offs
	       : block_width);
	  unsigned cur_block_height
	    = ((block_y_offs + block_height > output.height)
	       ? output.height - block_y_offs
	       : block_height);

	  renderer.render_block (offs_x + block_x_offs, offs_y + block_y_offs,
				 cur_block_width, cur_block_height);

	  prog.update (cur_block_num++);
      }

      output.flush ();
    }

  stats = renderer.trace_stats ();

  prog.end ();
}



// Return an appropriate sample generator.
//
static Sample2Gen *
make_sample_gen (const Params &params)
{
  unsigned oversample = params.get_uint ("oversample", 1);
  unsigned jitter = params.get_uint ("jitter", 0);
  return new Grid (oversample, oversample, jitter);
}

void
Snogray::render (const Scene &scene, const Camera &camera,
		 unsigned width, unsigned height,
		 ImageOutput &output, unsigned offs_x, unsigned offs_y,
		 const Params &params, TraceStats &stats,
		 std::ostream &progress_stream, Progress::Verbosity verbosity)
{
  std::auto_ptr<Sample2Gen> sample_gen (make_sample_gen (params));
  TraceParams trace_params (params);

  if (params.get_int ("render-by-rows", 0))
    render_by_rows (scene, camera, width, height, output, offs_x, offs_y,
		    *sample_gen, trace_params, stats,
		    progress_stream, verbosity);
  else
    render_by_blocks (16, 16,
		      scene, camera, width, height, output, offs_x, offs_y,
		      *sample_gen, trace_params, stats,
		      progress_stream, verbosity);
}

// arch-tag: cbd1f440-1ebb-465b-a4e3-bd17777245f9
