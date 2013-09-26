// renderer.cc -- Output rendering object
//
//  Copyright (C) 2006-2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene/scene.h"
#include "scene/camera.h"
#include "material/media.h"
#include "render/sample-set.h"
#include "render-packet.h"

#include "renderer.h"


using namespace snogray;

Renderer::Renderer (const GlobalRenderState &_global_state,
		    const Camera &_camera,
		    unsigned _width, unsigned _height)
  : camera (_camera), width (_width), height (_height),
    context (_global_state),
    camera_samples (context.samples.add_channel<UV> ()),
    focus_samples (context.samples.add_channel<UV> ())
{
}



// Render a single packet.
//
void
Renderer::render_packet (RenderPacket &packet)
{
  SampleSet &samples = context.samples;

  SurfaceInteg &surface_integ = *context.surface_integ;
  Media media (context.default_medium);

  packet.results.clear ();

  // Maximum length of a camera-ray.  We make it long enough to reach
  // any point in the scene's bounding-box from the camera's position.
  //
  dist_t max_trace = (context.scene.bbox () + camera.pos).diameter ();

  for (std::vector<UV>::const_iterator pi = packet.pixels.begin ();
       pi != packet.pixels.end (); ++pi)
    {
      UV pixel = *pi;

      samples.generate ();

      for (unsigned snum = 0; snum < samples.num_samples; snum++)
	{
	  SampleSet::Sample sample (samples, snum);

	  UV camera_samp = sample.get (camera_samples);
	  UV focus_samp = sample.get (focus_samples);

	  // The X/Y coordinates of the sample we're rendering inside PIXEL.
	  //
	  UV coords (pixel.u + camera_samp.u, pixel.v + camera_samp.v);

	  // Calculate the location on the film-plane (we flip the vertical
	  // coordinate because the output image has zero at the top,
	  // whereas rendering coordinates use zero at the bottom).
	  //
	  UV film_loc (coords.u / width, (height - coords.v) / height);

	  // Translate the image position U, V into a ray coming from the
	  // camera.
	  //
	  Ray camera_ray = camera.eye_ray (film_loc, focus_samp, max_trace);

	  // .. calculate what light arrives via that ray.
	  //
	  Tint tint = surface_integ.Li (camera_ray, media, sample);

	  packet.results.push_back (RenderPacket::Result (coords, tint));

	  context.mempool.reset ();
	}
    }
}


// arch-tag: 4c2c754d-4caa-487d-acd2-04bf97d849d3
