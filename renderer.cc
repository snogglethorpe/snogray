// renderer.cc -- Output rendering object
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "camera.h"
#include "filter.h"
#include "sample2-gen.h"

#include "renderer.h"

using namespace Snogray;

Renderer::Renderer (const Scene &_scene, const Camera &_camera,
		    unsigned _width, unsigned _height,
		    ImageOutput &_output, unsigned _offs_x, unsigned _offs_y,
		    unsigned max_y_block_size, Sample2Gen &_sample_gen,
		    const TraceParams &trace_params)
  : scene (_scene), camera (_camera), width (_width), height (_height),
    output (_output),
    lim_x (_offs_x), lim_y (_offs_y),
    lim_w (_output.width), lim_h (_output.height),
    sample_gen (_sample_gen), global_tstate (trace_params)
{
  output.set_num_buffered_rows (max_y_block_size);
}

// Render a block of pixels between X,Y and X+W,Y+H.  The coordinates
// are clamped to fit the global rendering limit.
// Note that blocks must be rendered in an order which fits within the
// output's row buffering.
//
void
Renderer::render_block (int x, int y, int w, int h)
{
  // Clamp the parameters to fit our limit (LIM_X, LIM_Y, LIM_W, LIM_H)

  if (y < lim_y)
    {
      h -= (lim_y - y);
      y = lim_y;
    }

  if (y + h > lim_y + lim_h)
    h = lim_y + lim_h - y;

  if (x < lim_x)
    {
      w -= (lim_x - x);
      x = lim_x;
    }

  if (x + w > lim_x + lim_w)
    w = lim_x + lim_w - x;

  // Now if there's anything left after clamping, render it
  //
  if (h > 0 && w > 0)
    {
      // If rendering the first or last row, and the output filter we're
      // using covers more than a single pixel, we must do a number of rows
      // preceding/following the first/last row for their effect on
      // following/preceding rows.
      //
      if (output.filter_radius != 0 && y == lim_y)
	{
	  y -= output.filter_radius;
	  h += output.filter_radius;
	}
      if (output.filter_radius != 0 && y + h == lim_y + lim_h)
	h += output.filter_radius;

      // Do the same thing for columns.
      //
      if (output.filter_radius != 0 && x == lim_x)
	{
	  x -= output.filter_radius;
	  w += output.filter_radius;
	}
      if (output.filter_radius != 0 && x + w == lim_x + lim_w)
	w += output.filter_radius;

      // This is basically a cache to speed up tracing by holding hints
      // that take advantage of spatial coherency.  We create a new one
      // for each block as the state at the end of the previous block is
      // probably not too useful anyway.
      //
      Trace trace (scene, global_tstate);

      // Render the desired rows row by row, and pixel by pixel
      //
      for (int py = y; py < y + h; py++)
	for (int px = x; px < x + w; px++)
	  render_pixel (px, py, trace);
    }
}

// Render a single output pixel at X,Y.  X and Y will be correctly
// handled even when they're outside the global rendering limit (such
// out-of-bounds pixels may still affect the output, because they are
// included in an in-bound pixel by the output filter).
//
void
Renderer::render_pixel (int x, int y, Trace &trace)
{
  // Generate samples within the pixel
  //
  sample_gen.generate ();

  for (Sample2Gen::iterator s = sample_gen.begin(); s != sample_gen.end(); s++)
    {
      // The X/Y coordinates of the specific sample S
      //
      float sx = x + s->u, sy = y + s->v;

      // Calculate normalized image coordinates U and V (we flip the V
      // coordinate vertically because the output image has zero at the
      // top, whereas rendering coordinates use zero at the bottom).
      //
      float u = sx / width, v = (height - sy) / height;

      // Translate the image position U, V into a ray coming from the
      // camera.
      //
      Ray camera_ray = camera.get_ray (u, v);

      // Cast the camera ray and calculate image color at that
      // point.
      //
      Color color = trace.render (camera_ray);

      // If necessary undo any bogus gamma-correction embedded in the scene
      // lighting.  Proper gamma correction will be done by the image
      // back-end when appropriate.
      //
      if (scene.assumed_gamma != 1)
	color = color.pow (scene.assumed_gamma);

      output.add_sample (sx - lim_x, sy - lim_y, color);
    }
}


// arch-tag: 4c2c754d-4caa-487d-acd2-04bf97d849d3
