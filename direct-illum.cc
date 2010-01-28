// direct-illum.cc -- Direct-lighting calculations
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "brdf.h"
#include "light.h"

#include "direct-illum.h"


using namespace snogray;



// mis_sample_weight ("power heuristic")

// Return a weight for one sample in multiple importantance sampling
// with two sample distributions.  PDF is the pdf for the sample being
// weighted, and NUM_SAMPLES is the number of samples taken from its
// distribution.  OTHER_PDF and NUM_OTHER_SAMPLES are the corresponding
// values from the other distribution being combined with this one via
// MIS.
//
static float
mis_sample_weight (float pdf, float num_samples,
		   float other_pdf, float num_other_samples)
{
  // This weight is calculated using the "power heuristic", with an
  // exponent of 2.

  float term = num_samples * pdf;
  float other_term = num_other_samples * other_pdf;

  // Handle near-degenerate cases that cause problems because of
  // floating-point overflow.
  //
  if (term > 1e10)
    return 1;
  else if (other_term > 1e10)
    return 0;

  // Raise the terms to a power of 2.
  //
  term *= term;
  other_term *= other_term;

  return term / (term + other_term);
}



DirectIllum::GlobalState::GlobalState (const ValTable &params)
  : num_light_samples (params.get_uint ("light-samples", 16))
{
}

DirectIllum::DirectIllum (RenderContext &context, GlobalState &global_state)
  : light_select_chan (context.samples.add_channel<float> ())
{
  SampleSet &samples = context.samples;
  unsigned num_lights = context.scene.num_lights ();
  unsigned num_lsamples = global_state.num_light_samples;

  for (unsigned i = 0; i < num_lights; i++)
    {
      light_samp_channels.push_back (samples.add_channel<UV> (num_lsamples));
      brdf_samp_channels.push_back (samples.add_channel<UV> (num_lsamples));
      brdf_layer_channels.push_back (samples.add_channel<float> (num_lsamples));
    }  
}


// DirectIllum::sample_all_lights

// Given an intersection resulting from a cast ray, sample all lights
// in the scene, and return the sum of their contribution in that
// ray's direction.
//
Color
DirectIllum::sample_all_lights (const Intersect &isec,
				const SampleSet::Sample &sample)
  const
{
  RenderContext &context = isec.context;
  unsigned num_lights = context.scene.lights.size ();

  context.stats.illum_calls++;

  Color radiance = 0;

  for (unsigned i = 0; i < num_lights; i++)
    {
      const Light *light = context.scene.lights[i];
      const SampleSet::Channel<UV> &light_chan = light_samp_channels[i];
      const SampleSet::Channel<UV> &brdf_chan = brdf_samp_channels[i];
      const SampleSet::Channel<float> &brdf_layer_chan = brdf_layer_channels[i];
      unsigned num_samples = light_chan.size;

      std::vector<UV>::const_iterator li = sample.begin (light_chan);
      std::vector<UV>::const_iterator bi = sample.begin (brdf_chan);
      std::vector<float>::const_iterator bli = sample.begin (brdf_layer_chan);

      Color light_radiance = 0;
      for (unsigned j = 0; j < num_samples; j++)
	light_radiance += estimate_direct (isec, light, *li++, *bi++, *bli++);

      radiance += light_radiance / float (num_samples);
    }

  return radiance;
}


// DirectIllum::estimate_direct

// Use multiple-importance-sampling to estimate the radiance of LIGHT
// towards ISEC, using SAMPLE, LIGHT_PARAM, BRDF_PARAM, and
// BRDF_LAYER_PARAM to sample both the light and the BRDF.
//
Color
DirectIllum::estimate_direct (const Intersect &isec, const Light *light,
			      const UV &light_param,
			      const UV &brdf_param, float brdf_layer_param)
  const
{
  RenderContext &context = isec.context;
  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  // Final result, which will be the sum of one light sample and one
  // BRDF sample.
  //
  Color radiance = 0;

  //
  // First, sample the light.
  //

  // Sample LIGHT, based on LIGHT_PARAM.
  //
  Light::Sample lsamp = light->sample (isec, light_param);

  if (lsamp.pdf > 0 && lsamp.val > 0)
    {
      // Now evaulate the BRDF in the direction of the light sample.
      //
      Brdf::Value bval = isec.brdf->eval (lsamp.dir);

      if (bval.val > 0)
	{
	  // Now we know there's a potential contribution, so check to
	  // see if this sample is occluded or not.

	  //
	  // XXX Should encapsulate the standard grot surrounding
	  // shadow-testing (horizon distance, into some convenience
	  // class... XXX
	  // (e.g., PBRT's "VisibilityTester" class?)
	  //
	  dist_t max_dist = lsamp.dist ? lsamp.dist - min_dist : scene.horizon;
 
	  ShadowRay ray (isec, isec.normal_frame.from (lsamp.dir),
			 min_dist, max_dist);
 
	  if (! scene.intersects (ray, context))
	    {
	      // The sample is not occluded, calculate the actual radiance.

	      Color lsamp_radiance = lsamp.val;

	      lsamp_radiance
		*= context.volume_integ->transmittance (ray, isec.trace.medium);

	      // Apply the "power heuristic" to weight our sample based
	      // on the relative pro
	      //
	      if (! light->is_point_light ())
		lsamp_radiance
		  *= mis_sample_weight (lsamp.pdf, 1, bval.pdf, 1);

	      // Filter the light through the BRDF function.
	      //
	      lsamp_radiance *= bval.val;

	      // Apply cos theta term.
	      //
	      lsamp_radiance *= abs (isec.cos_n (lsamp.dir));

	      lsamp_radiance /= lsamp.pdf;

	      radiance += lsamp_radiance;
 	    }
	}
    }

  //
  // Next, sample the BRDF.
  //

  // We only sample using the BRDF if the light isn't a point-light
  // (with a point light, the probability that the light will exactly
  // coincide with a chosen BRDF sample direction is zero, so it's
  // pointless).
  //
  if (! light->is_point_light ())
    {
      // Sample the BRDF, based on BRDF_PARAM.
      // We avoid specular samples, as they should be handled elsewhere.
      //
      Brdf::Sample bsamp
	= isec.brdf->sample (brdf_param, Brdf::ALL & ~Brdf::SPECULAR);

      if (bsamp.pdf > 0 && bsamp.val > 0)
	{
	  // Now evaluate the light in the direction of the BRDF sample.
	  //
	  Light::Value lval = light->eval (isec, bsamp.dir);

	  if (lval.pdf > 0 && lval.val > 0)
	    {
	      // Now we know there's a potential contribution, so check to
	      // see if this sample is occluded or not.

	      //
	      // XXX Should encapsulate the standard grot surrounding
	      // shadow-testing (horizon distance, into some convenience
	      // class... XXX
	      // (e.g., PBRT's "VisibilityTester" class?)
	      //
	      dist_t max_dist
		= lval.dist ? lval.dist - min_dist : scene.horizon;

	      ShadowRay ray (isec, isec.normal_frame.from (bsamp.dir),
			     min_dist, max_dist);

	      if (! scene.intersects (ray, context))
		{
		  // The sample is not occluded

		  Color bsamp_radiance = lval.val;

		  bsamp_radiance
		    *= context.volume_integ->transmittance (ray,
							    isec.trace.medium);

		  // Apply the "power heuristic" to weight our sample based
		  // on the relative pro
		  //
		  bsamp_radiance
		    *= mis_sample_weight (bsamp.pdf, 1, lval.pdf, 1);

		  // Filter the light through the BRDF function.
		  //
		  bsamp_radiance *= bsamp.val;

		  // Apply cos theta term.
		  //
		  bsamp_radiance *= abs (isec.cos_n (bsamp.dir));

		  bsamp_radiance /= bsamp.pdf;

		  radiance += bsamp_radiance;
		}
	    }
	}
    }

  return radiance;
}
