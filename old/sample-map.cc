// sample-map.cc -- Visual representation of sample distribution
//
//  Copyright (C) 2005-2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "ray.h"
#include "scene.h"
#include "camera.h"
#include "media.h"
#include "illum-mgr.h"
#include "spherical-coords.h"
#include "render-context.h"

#include "sample-map.h"

using namespace snogray;


// Add samples from the first intersection reached by tracing EYE_RAY
// into SCENE.
//
unsigned
SampleMap::sample (const Ray &eye_ray, Scene &scene,
		   const RenderParams &render_params,
		   const IllumMgr &illum_mgr)
{
  Ray intersected_ray (eye_ray, Scene::DEFAULT_HORIZON);

  RenderContext render_context (scene, render_params);

  const Surface::Renderable::IsecInfo *isec_info
    = scene.intersect (intersected_ray, context);

  if (isec_info)
    {
      Media media (isec_info->ray, render_context);
      Intersect isec = isec_info->make_intersect (media, context);
      unsigned num = illum_mgr.gen_samples (isec, samples);

      for (IllumSampleVec::iterator s = samples.begin() + num_samples;
	   s != samples.end(); ++s)
	{	
	  sum += s->light_val;
	  if (num_samples == 0 || s->light_val < min)
	    min = s->light_val;
	  if (s->light_val > max)
	    max = s->light_val;

	  num_samples++;
	}

      return num;
    }
  else
    return 0;
}

// Normalize samples
//
void
SampleMap::normalize ()
{
  float scale = 1 / max.intensity ();

  for (IllumSampleVec::iterator s = samples.begin (); s != samples.end (); s++)
    s->light_val *= scale;
}

// Draw a picture of the samples to MAP.  RADIUS is how wide a circle to use
// for drawing each sample; if RADIUS is zero then each sample is drawn with
// a single pixel in MAP.  COLOR is a color in which to draw the samples; if
// omitted (or negative), the actual color of the sample will be used.
//
void
SampleMap::draw (Image &map, unsigned radius, Color color)
{
  unsigned w = map.width, h = map.height;
  int rsq = radius * radius;

  bool use_sample_color = (color < 0);

  // We do two passes, the first to clear the samples (in case there's
  // already a background image in MAP), and the second to draw them.
  //
  for (unsigned pass = 0; pass < 2; pass++)
    for (IllumSampleVec::const_iterator s = samples.begin ();
	 s != samples.end (); s++)
      {  
	unsigned x = unsigned (w * (y_axis_longitude (s->dir) + PI) / (PI * 2));
	unsigned y = unsigned (h * (-y_axis_latitude (s->dir) + PI/2) / PI);

	Color col = use_sample_color ? s->light_val : color;

	for (int yi = -radius; yi <= int (radius); yi++)
	  for (int xi = -radius; xi <= int (radius); xi++)
	    if (xi*xi + yi*yi <= rsq)
	      {
		unsigned ox = (x + xi + w) % w, oy = (y + yi + h) % h;
		if (pass == 0)
		  map.put (ox, oy, 0);
		else
		  map.put (ox, oy, map.get (ox, oy) + col);
	      }
      }
}


// arch-tag: 1dd56b0e-9b6f-4918-8111-381692268f98
