// sample-map.cc -- Visual representation of sample distribution
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
#include "trace.h"
#include "mis-illum.h"
#include "global-tstate.h"

#include "sample-map.h"

using namespace snogray;
using namespace std;

// Add samples from the first intersection reached by tracing EYE_RAY
// into SCENE.
//
unsigned
SampleMap::sample (const Ray &eye_ray, Scene &scene,
		   const TraceParams &trace_params)
{
  Ray intersected_ray (eye_ray, Scene::DEFAULT_HORIZON);

  GlobalTraceState global_tstate (trace_params);
  Trace trace (scene, global_tstate);

  const Surface::IsecInfo *isec_info = scene.intersect (intersected_ray, trace);
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (intersected_ray, trace);
      MisIllum mis_illum (trace);

      mis_illum.distribute_light_samples (trace_params.num_light_samples,
					  mis_illum.light_params);
      mis_illum.gen_samples (isec, trace_params.num_brdf_samples,
			     mis_illum.light_params, samples);

      for (IllumSampleVec::iterator s = samples.begin() + num_samples;
	   s != samples.end(); ++s)
	{	
	  sum += s->val;
	  if (num_samples == 0 || s->val < min)
	    min = s->val;
	  if (s->val > max)
	    max = s->val;

	  num_samples++;
	}

      // Number of real+virtual samples.
      //
      unsigned num = 0;

      // Count the number of real+virtual light samples.
      //
      for (std::vector<SampleIllum::LightParams>::iterator lp
	     = mis_illum.light_params.begin();
	   lp != mis_illum.light_params.end(); ++lp)
	num += lp->num_samples;

      // There's no way to do the same thing for brdf samples, so assume
      // there are no virtual brdf samples (which is true more often
      // than for lights at least).
      //
      num += trace_params.num_brdf_samples;

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
    s->val *= scale;
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
	unsigned x = unsigned (w * (s->dir.longitude() + PI) / (PI * 2));
	unsigned y = unsigned (h * (-s->dir.latitude() + PI/2) / PI);

	Color col = use_sample_color ? s->val : color;

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
