// sample-map.cc -- Visual representation of sample distribution
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
		   const TraceParams &trace_params, bool intensity)
{
  Ray intersected_ray (eye_ray, Scene::DEFAULT_HORIZON);

  GlobalTraceState global_tstate (trace_params);
  Trace trace (scene, global_tstate);

  IsecParams isec_params;
  const Surface *surf = scene.intersect (intersected_ray, isec_params, trace);

  if (surf)
    {
      Intersect isec
	= surf->intersect_info (intersected_ray, isec_params, trace);

      MisIllum mis_illum (trace);

      mis_illum.distribute_light_samples (trace_params.num_light_samples,
					  mis_illum.light_params);
      mis_illum.gen_samples (isec, trace_params.num_brdf_samples,
			     mis_illum.light_params, samples);

      for (IllumSampleVec::iterator s = samples.begin() + num_samples;
	   s != samples.end(); ++s)
	{	
	  Color val = intensity ? s->val : 5;

	  put (s->dir, get (s->dir) + val);

	  if (intensity)
	    {
	      sum += val;
	      if (num_samples == 0 || val < min)
		min = val;
	      if (val > max)
		max = val;
	    }

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

  for (IllumSampleVec::const_iterator s = samples.begin ();
       s != samples.end (); s++)
    put (s->dir, get (s->dir) * scale);
}


// arch-tag: 1dd56b0e-9b6f-4918-8111-381692268f98
