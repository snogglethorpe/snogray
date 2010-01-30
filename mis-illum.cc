// mis-illum.cc -- Direct illumination using multiple importance sampling
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "light.h"
#include "brdf.h"
#include "intersect.h"

#include "mis-illum.h"


using namespace snogray;


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

// Return outgoing radiance for this illuminator.  The BRDF samples
// between BRDF_SAMPLES_BEG and BRDF_SAMPLES_END are matched to this
// illuminator.  NUM_BRDF_SAMPLES is the total number of non-specular
// BRDF samples generated (even those not passed to this illuminator).
//
// ILLUM_MGR can be used for recursively calculating illumination.
//
Color
MisIllum::Lo (const Intersect &isec,
	      const IllumSampleVec::iterator &brdf_samples_beg,
	      const IllumSampleVec::iterator &brdf_samples_end,
	      unsigned num_brdf_samples,
	      const IllumMgr &, unsigned, float complexity)
  const
{
  const std::vector<const Light *> &lights = scene.lights;
  unsigned num_lights = lights.size ();

  RenderContext &context = isec.context;

  unsigned global_light_samples = context.params.max_light_samples;
  unsigned local_light_samples = context.params.num_light_samples;
  unsigned desired_light_samples
    = clamp (unsigned (global_light_samples / complexity + 0.5f),
	     1u, local_light_samples);
  LightParamsVec light_params (desired_light_samples, lights, isec);

  // STL allocator for allocating samples.
  //
  IllumSampleVecAlloc samp_alloc (isec.mempool ());

  // Vector of light samples.
  //
  IllumSampleVec light_samples (samp_alloc);

  // Total outgoing light calculated.
  //
  Color radiance = 0;

  // Weight due to number of BRDF samples.
  //
  float brdf_sample_weight = num_brdf_samples ? 1.f / num_brdf_samples : 1.f;

  // Iterate over all lights, adding the contribution from each one.
  //
  for (unsigned lnum = 0; lnum < num_lights; lnum++)
    {
      const Light *light = lights[lnum];
      const LightParams &lparams = light_params[lnum];

      // We re-use LIGHT_SAMPLES for each light, so clear out any old contents.
      //
      light_samples.clear ();

      // Number of light samples taken for this light (possibly
      // including any implicit samples not actually present in the
      // LIGHT_SAMPLES vector).
      //
      unsigned num_light_samples = 0;

      // Generate the samples.
      //
      if (lparams.num_samples != 0)
	num_light_samples
	  = light->gen_samples (isec, lparams.num_samples, light_samples);

      context.stats.illum_samples += num_light_samples;

      // Process light samples for this light.
      //
      if (! light_samples.empty ())
	{
	  // Beginning/end of this light's samples.
	  //
	  const IllumSampleVec::iterator ls_beg = light_samples.begin ();
	  const IllumSampleVec::iterator ls_end = light_samples.end ();

	  // Calculate BRDF response for light samples from this light.
	  //
	  isec.brdf->filter_samples (ls_beg, ls_end);

	  // Shadow-test light samples.
	  //
	  shadow_test (isec, ls_beg, ls_end);

	  // Amount each sample counts in the final overall illumination
	  // equation (including all samples from all lights).
	  //
	  float light_sample_weight = lparams.weight / num_light_samples;

	  // We handle point lights specially, as they don't use any BRDF
	  // samples, and can't use multiple importance sampling.
	  //
	  if (lparams.is_point_light)
	    //
	    // Point light
	    //
	    for (IllumSampleVec::iterator s = ls_beg; s != ls_end; ++s)
	      radiance += s->light_val * s->brdf_val * light_sample_weight;

	  else
	    //
	    // Area light
	    //
	    for (IllumSampleVec::iterator s = ls_beg; s != ls_end; ++s)
	      {
		// Weight to adjust for multiple importance sampling
		//
		float mis_weight
		  = mis_sample_weight (s->light_pdf, num_light_samples,
				       s->brdf_pdf, num_brdf_samples);

		radiance
		  += (s->light_val * s->brdf_val
		      * mis_weight * light_sample_weight
		      * abs (isec.cos_n (s->dir))
		      / s->light_pdf);
	      }
	}

      // Process BRDF samples related to this light.  The light-related
      // fields for every sample have been filled in by our caller.
      //
      // We simply ignore any BRDF samples which don't hit our light, as
      // they will contribute nothing to this light's terms (so each
      // BRDF sample only contributes to illumination from a single
      // light, even though it's "counted" for all lights).
      //
      if (! lparams.is_point_light)
	{
	  float sample_weight = lparams.weight * brdf_sample_weight;

	  for (IllumSampleVec::iterator s = brdf_samples_beg;
	       s != brdf_samples_end; ++s)
	    if (s->light == light && (s->flags & IllumSample::DIRECT))
	      {
		Color val = s->light_val * s->brdf_val;

		// We can only do MIS for non-specular samples (specular
		// samples just get the value unmodified.
		//
		if (! (s->flags & IllumSample::SPECULAR))
		  {
		    // Weight to adjust for multiple importance sampling
		    //
		    float mis_weight
		      = mis_sample_weight (s->brdf_pdf, num_brdf_samples,
					   s->light_pdf, num_light_samples);

		    val *= mis_weight * sample_weight / s->brdf_pdf;
		    val *= abs (isec.cos_n (s->dir));
		  }

		radiance += val;
	      }
	}
    }

  return radiance;
}


// arch-tag: d7180220-e119-460b-8ae3-42a6426078e0
