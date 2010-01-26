// direct-integ.cc -- Direct-lighting-only surface integrator
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

#include "direct-integ.h"


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



DirectInteg::GlobalState::GlobalState (const Scene &scene,
				       const ValTable &params)
  : SurfaceInteg::GlobalState (scene),
    num_light_samples (params.get_uint ("light-samples", 16))
{
}

// Integrator state for rendering a group of related samples.
//
DirectInteg::DirectInteg (RenderContext &context, GlobalState &global_state)
  : SurfaceInteg (context),
    global (global_state),
    num_lights (context.scene.num_lights ()),
    light_select_chan (context.samples.add_channel<float> ())
{
  SampleSet &samples = context.samples;
  unsigned num_lsamples = global.num_light_samples;

  for (unsigned i = 0; i < num_lights; i++)
    {
      light_samp_channels.push_back (samples.add_channel<UV> (num_lsamples));
      brdf_samp_channels.push_back (samples.add_channel<UV> (num_lsamples));
      brdf_layer_channels.push_back (samples.add_channel<float> (num_lsamples));
    }  
}


// Return a new integrator, allocated in context.
//
SurfaceInteg *
DirectInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new DirectInteg (context, *this);
}


// DirectInteg::sample_all_lights

// Given an intersection resulting from a cast ray, sample all lights
// in the scene, and return the sum of their contribution in that
// ray's direction.
//
Color
DirectInteg::sample_all_lights (const Intersect &isec,
				const SampleSet::Sample &sample)
  const
{
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


// DirectInteg::estimate_direct

// Use multiple-importance-sampling to estimate the radiance of LIGHT
// towoards ISEC, using SAMPLE, LIGHT_PARAM, BRDF_PARAM, and
// BRDF_LAYER_PARAM to sample both the light and the BRDF.
//
Color
DirectInteg::estimate_direct (const Intersect &isec, const Light *light,
			      const UV &light_param,
			      const UV &brdf_param, float brdf_layer_param)
  const
{
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


// DirectInteg::Lo

// Return the color emitted from the ray-surface intersection ISEC.
// "Lo" means "Light outgoing".
//
// This an internal variant of Integ::lo which has an additional DEPTH
// argument.  If DEPTH is greater than some limit, recursion will stop.
//
Color
DirectInteg::Lo (const Intersect &isec, const SampleSet::Sample &sample,
		 unsigned depth)
  const
{
  // Start out by including any light emitted from the material itself.
  //
  Color radiance = isec.material->le (isec);

  // Now if there's a BRDF, add contributions from incoming light
  // reflected-from / transmitted-through the surface.  [Only weird
  // materials like light-emitters don't have a BRDF.]
  //
  if (isec.brdf)
    {
      // Include non-specular direct lighting.
      //
      radiance += sample_all_lights (isec, sample);

      //
      // If the BRDF includes specular components, recurse to handle those.
      //
      // Note that because there's only one possible specular sample for
      // each direction, we just pass a dummy (0,0) parameter to
      // Brdf::sample.
      //

      // Try reflection.
      //
      Brdf::Sample refl_samp
	= isec.brdf->sample (UV(0,0), Brdf::SPECULAR|Brdf::REFLECTIVE);
      if (refl_samp.val > 0)
	radiance
	  += (Li (isec, refl_samp.dir, sample,
		  Trace::REFLECTION, 0, depth + 1)
	      * refl_samp.val
	      * abs (isec.cos_n (refl_samp.dir)));

      // Try refraction.
      //
      Brdf::Sample xmit_samp
	= isec.brdf->sample (UV(0,0), Brdf::SPECULAR|Brdf::TRANSMISSIVE);
      if (xmit_samp.val > 0)
	{
	  Trace::Type subtrace_type;
	  const Medium *subtrace_medium;

	  if (isec.back)
	    {
	      subtrace_type = Trace::REFRACTION_OUT;
	      subtrace_medium
		= &isec.trace.enclosing_medium (context.default_medium);
	    }
	  else
	    {
	      subtrace_type = Trace::REFRACTION_IN;
	      subtrace_medium = isec.material->medium ();
	    }

	  radiance
	    += (Li (isec, xmit_samp.dir, sample,
		    subtrace_type, subtrace_medium,
		    depth + 1)
		* xmit_samp.val
		* abs (isec.cos_n (xmit_samp.dir)));
	}
    }

  return radiance;
}


// DirectInteg::Li

// Return the light hitting TARGET_ISEC from direction DIR; DIR is in
// TARGET_ISEC's surface-normal coordinate-system.  SUBTRACE_TYPE and
// SUBTRACE_MEDIUM describe the type of transition represented by the new
// ray, and the medium it has entered.  If SUBTRACE_MEDIUM is 0, then
// TARGET_ISEC's medium is used instead.
//
Color
DirectInteg::Li (const Intersect &target_isec, const Vec &dir,
		 const SampleSet::Sample &sample,
		 Trace::Type subtrace_type,
		 const Medium *subtrace_medium,
		 unsigned depth)
  const
{
  if (depth > 5) return 0; // XXX use russian roulette

  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  Ray isec_ray (target_isec.normal_frame.origin,
		target_isec.normal_frame.from (dir),
		min_dist, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

  Color radiance;
  if (isec_info)
    {
      if (! subtrace_medium)
	subtrace_medium = &target_isec.trace.medium;

      Trace trace (subtrace_type, isec_info->ray, *subtrace_medium, 1.f,
		   target_isec.trace);

      Intersect isec = isec_info->make_intersect (trace, context);

      radiance = Lo (isec, sample, depth);
    }
  else
    radiance = scene.background_with_alpha (isec_ray).alpha_scaled_color();

  radiance *= context.volume_integ->transmittance (isec_ray,
						   context.default_medium);

  radiance += context.volume_integ->Li (isec_ray,
					context.default_medium,
					sample);

  return radiance;
}
