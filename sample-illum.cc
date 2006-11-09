// sample-illum.cc -- Sample-based illuminator
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <algorithm>

#include "trace.h"
#include "scene.h"

#include "sample-illum.h"


using namespace Snogray;



SampleIllum::SampleIllum (const Trace &trace)
  : lights (trace.scene.lights), num_lights (lights.size ()),
    light_params (num_lights),
    num_area_lights (0), num_point_lights (0)
{
  // Initialize LIGHT_PARAMS and other light-derived fields.

  for (unsigned lnum = 0; lnum < num_lights; lnum++)
    {
      light_params[lnum].is_point_light = lights[lnum]->is_point_light ();
      light_params[lnum].weight = 1;
      light_params[lnum].num_samples = 0; // should be set later

      if (light_params[lnum].is_point_light)
	num_point_lights++;
      else
	num_area_lights++;
    }
}



// Comparison class for sorting the sample indices by sample value
//
struct sample_val_gtr
{
  bool operator() (const IllumSample &s1, const IllumSample &s2) const
  {
    return s1.val > s2.val;
  }
};

// Return the illumination from the samples in SAMPLES.
//
Color
SampleIllum::eval_samples (const Intersect &isec, IllumSampleVec &samples)
{
  const Trace &trace = isec.trace;

  trace.global.stats.illum_samples += samples.size ();

  // Sort the samples into descending order by intensity.
  //
  std::sort (samples.begin (), samples.end (), sample_val_gtr ());

  // Compute total possible radiance from all samples.  Also see how many
  // specular samples there are.
  //
  Color poss_radiance;
  unsigned num_specular_samples = 0;
  for (IllumSampleVec::iterator s = samples.begin (); s != samples.end(); ++s)
    if (! s->invalid)
      {
	if (s->specular)
	  num_specular_samples++;
	else
	  poss_radiance += s->val;
      }

  // Accumulated output radiance
  //
  Color radiance;

  // Trace any specular samples.
  //
  if (num_specular_samples != 0)
    {
      Trace &spec_sub_trace = isec.subtrace (Trace::REFLECTION);

      for (IllumSampleVec::iterator s = samples.begin ();
	   num_specular_samples != 0 && s != samples.end (); ++s)
	if (!s->invalid && s->specular)
	  {
	    Ray spec_ray (isec.pos, s->dir);
	    Color val = s->refl * spec_sub_trace.render (spec_ray);

	    // For samples which are not truly specular (only "near
	    // specular"), we must adjust by the pdf of the sample.
	    //
	    if (s->brdf_pdf != 0)
	      val /= s->brdf_pdf;

	    // Add the resulting radiance to the total radiance.
	    //
	    radiance += val;

	    // Add to POSS_RADIANCE as well; the effect of doing this is to
	    // allow light from specular samples to "overwhelm"
	    // non-specular samples, so we can avoid evaluating the latter.
	    //
	    poss_radiance += val;
	  }
    }

  // Shoot shadow rays to check the visibility of the chosen set of
  // light samples, accumulating the resulting radiance in RADIANCE.
  //
  if (samples.size () != num_specular_samples)
    {
      trace.global.stats.illum_specular_samples += num_specular_samples;

      Trace &shadow_sub_trace = isec.subtrace (Trace::SHADOW);

      for (IllumSampleVec::iterator s = samples.begin ();
	   s != samples.end (); ++s)
	if (!s->invalid && s->val > 0 && !s->specular)
	  {
#if 0
	    // If RADIANCE is beyond some threshold, give up even though we
	    // haven't finished all samples yet.  This means that in cases
	    // where the output radiance is dominated by very bright lights, we
	    // won't waste time calculating the remaining dim ones.
	    //
	    if (radiance > poss_radiance * 0.95 /* XXX make a variable */)
	      {
		// XXX somehow use the unused samples, except without sending
		// out shadow rays [perhaps (1) keep track of last-known
		// visibility per-light, and (2) update the visibility of some
		// random subset of the remaining unused lights]
		//
		break;
	      }
#endif

	    dist_t max_dist = s->dist ? s->dist : trace.scene.horizon;
	    const Ray shadow_ray (isec.pos, s->dir, max_dist);

	    // Find any surface that's shadowing LIGHT_RAY.
	    //
	    const Surface *shadower
	      = shadow_sub_trace.shadow_caster (shadow_ray, isec, s->light);

	    if (! shadower)
	      //
	      // The surface is not shadowed at all, just add the light.
	      //
	      radiance += s->val;

	    else if (shadower->material->shadow_type != Material::SHADOW_OPAQUE)
	      //
	      // There's a shadower, but it's not opaque, so give it (and
	      // any further surfaces) a chance to attentuate the color.
	      {
		trace.global.stats.scene_slow_shadow_traces++;

		// The reflected radiance from this sample, after being
		// adjusted for the filtering of the source irradiance through
		// SHADOWER.
		//
		Color filtered
		  = shadow_sub_trace.shadow (shadow_ray, s->val, *s->light);

		// Add the final filtered radiance to RADIANCE, and also adjust
		// our estimation of the total possible radiance to account for
		// the filtering.
		//
		radiance += filtered;
		poss_radiance -= s->val - filtered;
	      }

	    else
	      // The surface is shadowed; subtract this light from the possible
	      // radiance.
	      //
	      poss_radiance -= s->val;
	  }
    }

  return radiance;
}



void
SampleIllum::distribute_light_samples (unsigned num,
				       std::vector<LightParams> &light_params)
{
  // For now, evenly divide the samples between lights; we really should do
  // something more intelligent though, like distribute according to
  // intensity and/or apparent angular size.

  // If there are enough samples, dedicate one to each point-light, and
  // then divide the remainder among non-point-lights.  Otherwise, we just
  // randomly allocate 0 or 1 sample to each light.
  //
  if (num > num_lights)
    {
      num -= num_point_lights;

      unsigned num_per_area_light = num / num_area_lights;
      unsigned num_left_over = num - num_area_lights * num_per_area_light;
      float left_over_frac = float (num_left_over) / float (num_per_area_light);

      // Generate samples from each area light.
      //
      for (std::vector<LightParams>::iterator lp = light_params.begin ();
	   lp != light_params.end (); ++lp)
	if (lp->is_point_light)
	  lp->num_samples = 1;
	else
	  {
	    lp->num_samples = num_per_area_light;

	    if (num_left_over > 0)
	      if (lp + 1 == light_params.end ())
		lp->num_samples += num_left_over;
	      else if (random (1.f) < left_over_frac)
		{
		  lp->num_samples++;
		  num_left_over--;
		}
	  }
    }
  else
    {
      float prob = float (num) / float (num_lights);

      for (std::vector<LightParams>::iterator lp = light_params.begin ();
	   lp != light_params.end (); ++lp)
	lp->num_samples = (random (1.f) < prob); // 0 or 1 sample
    }
}



// Illuminate the intersection ISEC.
//
Color
SampleIllum::illum (const Intersect &isec)
{
  const TraceParams &params = isec.trace.global.params;

  samples.clear ();

  // Decide how many samples to use for each light, placing the counts
  // into the array NUM_LIGHT_SAMPLES (indexed by light number).
  //
  distribute_light_samples (params.num_light_samples, light_params);

  // Generate the samples (from the BRDF and/or light).
  //
  gen_samples (isec, params.num_brdf_samples, light_params, samples);

  return eval_samples (isec, samples);
}


// arch-tag: 6faa08a1-7684-46c4-b401-7ea15f52c4f6
