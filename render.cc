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

  render_by_rows (scene, camera, width, height, output, offs_x, offs_y,
		  *sample_gen, trace_params, stats, progress_stream, verbosity);
}

// arch-tag: cbd1f440-1ebb-465b-a4e3-bd17777245f9
