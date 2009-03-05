// renderer.cc -- Output rendering object
//
//  Copyright (C) 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "camera.h"
#include "filter.h"
#include "sample2-gen.h"

#include "renderer.h"

using namespace snogray;

Renderer::Renderer (const Scene &_scene, const Camera &_camera,
		    unsigned _width, unsigned _height,
		    ImageOutput &_output, unsigned _offs_x, unsigned _offs_y,
		    unsigned max_y_block_size,
		    IllumMgr &_illum_mgr,
		    Sample2Gen &_sample_gen, Sample2Gen &_focus_sample_gen,
		    const TraceParams &trace_params)
  : scene (_scene), camera (_camera), width (_width), height (_height),
    illum_mgr (_illum_mgr),
    output (_output),
    lim_x (_offs_x), lim_y (_offs_y),
    lim_w (_output.width), lim_h (_output.height),
    sample_gen (_sample_gen), focus_sample_gen (_focus_sample_gen),
    trace_context (trace_params)
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
  // Clamp the parameters to fit our limit (LIM_X, LIM_Y, LIM_W, LIM_H).
  // We also clamp with respect to the physical output boundaries.

  int min_y = lim_y + output.min_y, max_y = lim_y + lim_h;
  int min_x = lim_x, max_x = lim_x + lim_w;

  if (y < min_y)
    {
      h -= (min_y - y);
      y = min_y;
    }
  if (y + h > max_y)
    h = max_y - y;

  if (x < min_x)
    {
      w -= (min_x - x);
      x = min_x;
    }
  if (x + w > max_x)
    w = max_x - x;

  // Now if there's anything left after clamping, render it
  //
  if (h > 0 && w > 0)
    {
      unsigned filt_rad = output.filter_radius ();

      // If rendering the first or last row, and the output filter we're
      // using covers more than a single pixel, we must do a number of rows
      // preceding/following the first/last row for their effect on
      // following/preceding rows.
      //
      if (filt_rad != 0 && y == min_y)
	{
	  y -= filt_rad;
	  h += filt_rad;
	}
      if (filt_rad != 0 && y + h == max_y)
	h += filt_rad;

      // Do the same thing for columns.
      //
      if (filt_rad != 0 && x == min_x)
	{
	  x -= filt_rad;
	  w += filt_rad;
	}
      if (filt_rad != 0 && x + w == max_x)
	w += filt_rad;

      // This is basically a cache to speed up tracing by holding hints
      // that take advantage of spatial coherency.  We create a new one
      // for each block as the state at the end of the previous block is
      // probably not too useful anyway.
      //
      Trace trace (scene, trace_context);

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

  // Generate samples for depth-of-field simulation.  There are the same
  // number as anti-alising samples, but their position should not be
  // correlated.
  //
  // We only need these samples when the camera's "aperture" is non-zero,
  // but for simplicity we always step through the samples, so we need to
  // generate the focus samples the first time even in the zero-aperture
  // case just to make sure the sample vector is full.
  //
  if (camera.aperture != 0 || focus_sample_gen.size () == 0)
    {
      focus_sample_gen.generate ();
      focus_sample_gen.shuffle (); // de-correlate from antialiasing samples
    }

  for (Sample2Gen::iterator s = sample_gen.begin(),
	 fs = focus_sample_gen.begin ();
       s != sample_gen.end(); ++s, ++fs)
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
      Ray camera_ray = camera.eye_ray (u, v, fs->u, fs->v);
      camera_ray.t1 = scene.horizon;

      //
      // Cast the camera ray and calculate image color from that direction.
      //

      Ray intersected_ray (camera_ray);
      const Surface::IsecInfo *isec_info
	= scene.intersect (intersected_ray, trace);

      Tint tint;
      if (isec_info)
	tint = illum_mgr.li (isec_info, trace);
      else
	tint = scene.background_with_alpha (camera_ray);

      trace.context.mempool.reset ();

      output.add_sample (sx - lim_x, sy - lim_y, tint);
    }
}


// arch-tag: 4c2c754d-4caa-487d-acd2-04bf97d849d3
