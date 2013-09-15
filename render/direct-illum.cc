// direct-illum.cc -- Direct-lighting calculations
//
//  Copyright (C) 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene/scene.h"
#include "material/bsdf.h"
#include "light/light.h"
#include "material/media.h"
#include "mis-sample-weight.h"

#include "direct-illum.h"


using namespace snogray;



// Constructor that allows explicitly setting the number of samples.
//
DirectIllum::GlobalState::GlobalState (unsigned _num_samples)
  : num_samples (_num_samples)
{
}


DirectIllum::DirectIllum (RenderContext &context,
			  const GlobalState &global_state)
  : num_lights_to_sample (
      global_state.num_samples == 0
      ? 0
      : context.scene.num_light_samplers ()),
    light_select_chan (context.samples.add_channel<float> ())
{
  finish_init (context.samples, global_state);
}

// Variant constructor which allows specifying a SampleSet other than the
// one in CONTEXT.
//
DirectIllum::DirectIllum (SampleSet &samples, RenderContext &context,
			  const GlobalState &global_state)
  : num_lights_to_sample (
      global_state.num_samples == 0
      ? 0
      : context.scene.num_light_samplers ()),
    light_select_chan (samples.add_channel<float> ())
{
  finish_init (samples, global_state);
}
    
// Common portion of constructors.
//
void
DirectIllum::finish_init (SampleSet &samples, const GlobalState &global_state)
{
  unsigned num_samples = global_state.num_samples;

  for (unsigned i = 0; i < num_lights_to_sample; i++)
    {
      light_samp_channels.push_back (samples.add_channel<UV> (num_samples));
      bsdf_samp_channels.push_back (samples.add_channel<UV> (num_samples));
      bsdf_layer_channels.push_back (samples.add_channel<float> (num_samples));
    }  
}


// DirectIllum::sample_all_lights

// Given the intersection ISEC, resulting from a cast ray, sample
// all lights in the scene, and return the sum of their contribution
// in that ray's direction.  FLAGS specifies what part of the BSDF
// will be used.
//
Color
DirectIllum::sample_all_lights (const Intersect &isec,
				const SampleSet::Sample &sample,
				unsigned flags)
  const
{
  RenderContext &context = isec.context;

  context.stats.illum_calls++;

  Color radiance = 0;

  for (unsigned i = 0; i < num_lights_to_sample; i++)
    {
      const Light::Sampler *light_sampler = context.scene.light_samplers[i];
      const SampleSet::Channel<UV> &light_chan = light_samp_channels[i];
      const SampleSet::Channel<UV> &bsdf_chan = bsdf_samp_channels[i];
      const SampleSet::Channel<float> &bsdf_layer_chan = bsdf_layer_channels[i];
      unsigned num_samples = light_chan.size;

      std::vector<UV>::const_iterator li = sample.begin (light_chan);
      std::vector<UV>::const_iterator bi = sample.begin (bsdf_chan);
      std::vector<float>::const_iterator bli = sample.begin (bsdf_layer_chan);

      Color light_radiance = 0;
      for (unsigned j = 0; j < num_samples; j++)
	light_radiance
	  += sample_light (isec, light_sampler, *li++, *bi++, *bli++, flags);

      radiance += light_radiance / float (num_samples);
    }

  return radiance;
}


// DirectIllum::sample_light

// Use multiple-importance-sampling to estimate the radiance of
// LIGHT_SAMPLER towards ISEC, LIGHT_PARAM, BSDF_PARAM, and
// BSDF_LAYER_PARAM to sample both the light and the BSDF.  FLAGS
// specifies what part of the BSDF will be used.
//
Color
DirectIllum::sample_light (const Intersect &isec,
			   const Light::Sampler *light_sampler,
			   const UV &light_param,
			   const UV &bsdf_param, float bsdf_layer_param,
			   unsigned flags)
  const
{
  RenderContext &context = isec.context;
  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  // Final result, which will be the sum of one light sample and one
  // BSDF sample.
  //
  Color radiance = 0;

  //
  // First, sample the light.
  //

  // Sample LIGHT, based on LIGHT_PARAM.
  //
  Light::Sampler::Sample lsamp = light_sampler->sample (isec, light_param);

  if (lsamp.pdf > 0 && lsamp.val > 0)
    {
      // Now evaulate the BSDF in the direction of the light sample.
      //
      Bsdf::Value bval = isec.bsdf->eval (lsamp.dir, flags);

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
 
	  Ray ray (isec.normal_frame.origin,
		   isec.normal_frame.from (lsamp.dir),
		   min_dist, max_dist);

	  Color transmittance = 1;
	  if (! scene.occludes (ray, isec.media.medium, transmittance, context))
	    {
	      // The sample is not occluded, calculate the actual radiance.

	      Color lsamp_radiance = lsamp.val * transmittance;

	      lsamp_radiance
		*= context.volume_integ->transmittance (ray, isec.media.medium);

	      // Apply the "power heuristic" to weight our sample based
	      // on the relative pro
	      //
	      if (! light_sampler->is_point_light ())
		lsamp_radiance
		  *= mis_sample_weight (lsamp.pdf, 1, bval.pdf, 1);

	      // Filter the light through the BSDF function.
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
  // Next, sample the BSDF.
  //

  // We only sample using the BSDF if the light isn't a point-light
  // (with a point light, the probability that the light will exactly
  // coincide with a chosen BSDF sample direction is zero, so it's
  // pointless).
  //
  if (! light_sampler->is_point_light ())
    {
      // Sample the BSDF, based on BSDF_PARAM.
      //
      Bsdf::Sample bsamp = isec.bsdf->sample (bsdf_param, flags);

      if (bsamp.pdf > 0 && bsamp.val > 0)
	{
	  // Now evaluate the light in the direction of the BSDF sample.
	  //
	  Light::Sampler::Value lval = light_sampler->eval (isec, bsamp.dir);

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

	      Ray ray (isec.normal_frame.origin,
		       isec.normal_frame.from (bsamp.dir),
		       min_dist, max_dist);

	      Color transmittance = 1;
	      if (! scene.occludes (ray, isec.media.medium, transmittance,
				    context))
		{
		  // The sample is not occluded

		  Color bsamp_radiance = lval.val * transmittance;

		  bsamp_radiance
		    *= context.volume_integ->transmittance (ray,
							    isec.media.medium);

		  // Apply the "power heuristic" to weight our sample based
		  // on the relative pro
		  //
		  bsamp_radiance
		    *= mis_sample_weight (bsamp.pdf, 1, lval.pdf, 1);

		  // Filter the light through the BSDF function.
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
