// sample-map.cc -- Visual representation of sample distribution
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "ray.h"
#include "scene.h"
#include "camera.h"
#include "trace.h"

#include "sample-map.h"

using namespace Snogray;
using namespace std;

// Add samples from the first intersection reached by tracing EYE_RAY
// into SCENE.
//
void
SampleMap::sample (const Ray &eye_ray, Scene &scene)
{
  Ray intersected_ray (eye_ray, Scene::DEFAULT_HORIZON);

  GlobalTraceState global_tstate;
  Trace trace (scene, global_tstate);

  IsecParams isec_params;
  const Surface *surf = scene.intersect (intersected_ray, isec_params, trace);

  if (surf)
    {
      Intersect isec
	= surf->intersect_info (intersected_ray, isec_params, trace);
      std::vector<Light *> &lights = scene.lights;

      if (map_type == BRDF)
	isec.brdf.gen_samples (isec, samples);
      else if (map_type == LIGHTS)
	for (std::vector<Light *>::const_iterator li = lights.begin();
	     li != lights.end(); li++)
	  (*li)->gen_samples (isec, samples);
      else
	{
	  LightSamples &lsamples = global_tstate.light_samples;

	  lsamples.generate (isec, lights);

	  // This loop should be the same as the following two methods, but
	  // they run into memory allocation botches...
	  //
	  for (LightSamples::iterator ls = lsamples.begin();
	       ls != lsamples.end(); ls++)
	    samples.add (*ls);

	  // XXX1: copy (lsamples.begin(), lsamples.end(), samples.end());
	  // XXX2: samples.insert (samples.end(), lsamples.begin(), lsamples.end()); 
	}

      for (SampleRayVec::iterator s = samples.begin() + num_samples;
	   s != samples.end(); s++)
	{	
	  pixel (s->dir) += s->val;

	  sum += s->val;
	  if (num_samples == 0 || s->val < min)
	    min = s->val;
	  if (s->val > max)
	    max = s->val;

	  num_samples++;
	}
    }
}

// Normalize samples
//
void
SampleMap::normalize ()
{
  float scale = 1 / max.intensity ();

  for (SampleRayVec::const_iterator s = samples.begin ();
       s != samples.end (); s++)
    pixel (s->dir) *= scale;
}

// Save this map to a file.
//
void
SampleMap::save (const ImageSinkParams &params) const
{
  ImageGrrrSinkParams _params (params);
  
  _params.width = map.width;
  _params.height = map.height;

  map.save (_params);
}

// arch-tag: 1dd56b0e-9b6f-4918-8111-381692268f98
