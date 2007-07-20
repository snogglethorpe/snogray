// renderer.h -- Outer rendering loop
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "image-output.h"
#include "global-tstate.h"


namespace snogray {


class Scene;
class Camera;
class ImageSink;
class Filter;
class Sample2Gen;
class Trace;


class Renderer
{
public:

  Renderer (const Scene &_scene, const Camera &_camera,
	    unsigned _width, unsigned _height,
	    ImageOutput &_output, unsigned _offs_x, unsigned _offs_y,
	    unsigned max_y_block_size,
	    Sample2Gen &_sample_gen, Sample2Gen &_focus_sample_gen,
	    const TraceParams &trace_params);

  // Render a block of pixels between X,Y and X+W,Y+H.  The coordinates
  // are clamped to fit the global rendering limit.
  // Note that blocks must be rendered in an order which fits within the
  // output's row buffering, as determinted by MAX_Y_BLOCK_SIZE.
  //
  void render_block (int x, int y, int w, int h);

  // Render a single output pixel at X,Y.  X and Y will be correctly
  // handled even when they're outside the global rendering limit (such
  // out-of-bounds pixels may still affect the output, if they are included
  // in an in-bound pixel by the output filter).
  //
  void render_pixel (int x, int y, Trace &trace);

  TraceStats trace_stats () const { return global_tstate.stats; }


private:

  // The scene and camera being rendered.
  //
  const Scene &scene;
  const Camera &camera;

  // Size of the virtual screen being rendered to.  These are floats
  // because they are always used as such.
  //
  float width, height;

  // Where the results are written.  This may be smaller than the nominal
  // output image defined by WIDTH and HEIGHT.
  //
  ImageOutput &output;

  // The bounds of the actual output image (OUTPUT) within the virtual
  // screen defined by WIDTH and HEIGHT.
  //
  int lim_x, lim_y, lim_w, lim_h;

  // Sample generator used to generate samples within a pixel.
  //
  Sample2Gen &sample_gen;

  // Sample generator used to generate focus samples.
  //
  Sample2Gen &focus_sample_gen;

  // Global R/W state during tracing.
  //
  GlobalTraceState global_tstate;

#if 0
  // Wire-frame state.  We create this object regardless of whether
  // we're doing wire-frame output or not; in the latter case it simply
  // isn't used.
  //
  WireFrameRendering wire_frame_rendering;
#endif
};

}

#endif /* __RENDERER_H__ */

// arch-tag: d7ddcb87-cf77-4eb0-a558-a73e4aa4a75f
