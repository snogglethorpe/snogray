// direct-illum.cc -- Direct illumination superclass
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

#include "light.h"
#include "scene.h"
#include "shadow-ray.h"

#include "direct-illum.h"


using namespace snogray;


// Do shadow testing on the samples from BEG to END, and set their value
// to zero if shadowed.
//
void
DirectIllum::shadow_test (const Intersect &isec,
			  IllumSampleVec::iterator beg,
			  IllumSampleVec::iterator end)
  const
{
  Trace &trace = isec.trace;
  const Scene &scene = trace.scene;
  TraceContext &context = isec.context;

  dist_t min_dist = context.params.min_trace;

  // Info passed to intersection methods.
  //
  Surface::IsecCtx isec_ctx (isec);

  for (IllumSampleVec::iterator s = beg; s != end; ++s)
    if (s->light_val > 0)
      {
	dist_t max_dist = s->light_dist ? s->light_dist : scene.horizon;

	ShadowRay ray (isec, isec.normal_frame.from (s->dir),
		       min_dist, max_dist, s->light);

	if (scene.shadows (ray, isec_ctx))
	  s->light_val = 0;
      }
}


DirectIllum::LightParamsVec::LightParamsVec (
			       unsigned num_samples,
			       const std::vector<const Light *> &lights,
			       const Intersect &isec)
  : std::vector<LightParams, LightParamsVecAlloc>
    (LightParamsVecAlloc (isec.mempool ()))
{
  // For now, evenly divide the samples between lights; we really should do
  // something more intelligent though, like distribute according to
  // intensity and/or apparent angular size.

  unsigned num_lights = lights.size ();
  unsigned num_area_lights = 0, num_point_lights = 0;

  resize (lights.size ());

  // Initialize LIGHT_PARAMS and other light-derived fields.
  //
  for (unsigned lnum = 0; lnum < num_lights; lnum++)
    {
      LightParams &lparams = (*this) [lnum];

      lparams.is_point_light = lights[lnum]->is_point_light ();
      lparams.weight = 1;
      lparams.num_samples = 0; // should be set later

      if (lparams.is_point_light)
	num_point_lights++;
      else
	num_area_lights++;
    }

  // If there are enough samples, dedicate one to each point-light, and
  // then divide the remainder among non-point-lights.  Otherwise, we just
  // randomly allocate 0 or 1 sample to each light.
  //
  if (num_samples > num_lights)
    {
      if (num_area_lights == 0)
	{
	  for (iterator lp = begin (); lp != end (); ++lp)
	    lp->num_samples = 1;
	}
      else
	{
	  num_samples -= num_point_lights;

	  unsigned num_per_area_light = num_samples / num_area_lights;
	  unsigned num_left_over = num_samples - num_area_lights * num_per_area_light;
	  float left_over_frac
	    = float (num_left_over) / float (num_per_area_light);

	  // Generate samples from each area light.
	  //
	  for (iterator lp = begin (); lp != end (); ++lp)
	    if (lp->is_point_light)
	      lp->num_samples = 1;
	    else
	      {
		lp->num_samples = num_per_area_light;

		if (num_left_over > 0)
		  {
		    if (lp + 1 == end ())
		      lp->num_samples += num_left_over;
		    else if (random (1.f) < left_over_frac)
		      {
			lp->num_samples++;
			num_left_over--;
		      }
		  }
	      }
	}
    }
  else
    {
      float prob = float (num_samples) / float (num_lights);

      for (iterator lp = begin (); lp != end (); ++lp)
	lp->num_samples = (random (1.f) < prob); // 0 or 1 sample
    }
}
