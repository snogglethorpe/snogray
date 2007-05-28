// mis-illum.cc -- Illuminator using multiple importance sampling
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//
#include<iostream>

#include "intersect.h"
#include "light.h"

#include "mis-illum.h"


using namespace snogray;


#define MIS_VERBOSE_DEBUG 0


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

// Generate samples for estimating the illumination at ISEC.  The
// samples should be appended to SAMPLES.  NUM_BRDF_SAMPLES is the
// number of brdf samples to use, and LIGHT_PARAMS is an array of
// sampling parameters for each light (which may be modified).
//
void
MisIllum::gen_samples (const Intersect &isec,
		       unsigned num_brdf_samples,
		       std::vector<LightParams> &light_params,
		       IllumSampleVec &samples)
{
  // Generate BRDF samples.
  //
  num_brdf_samples = isec.brdf.gen_samples (isec, num_brdf_samples, samples);
  unsigned num_real_brdf_samples = samples.size ();

  // We mark any BRDF samples over a certain threshold, as "specular" (even
  // if they're not really specular, they're probably getting close).  For
  // these samples well do full subtraces instead of calculating the
  // illumination here.  We also calculate NUM_IMP_BRDF_SAMPLES, which is
  // the number of BRDF samples which are not specular, for which we want
  // to do importance sampling.
  //
//   float specular_threshold = isec.trace.global.params.specular_threshold;
  for (IllumSampleVec::iterator s = samples.begin(); s != samples.end (); ++s)
    {
//       if (s->refl > specular_threshold)
// 	s->specular = true;
      if (s->specular)
	num_brdf_samples--;
    }

  // Total number of samples for all lights, including both light samples
  // and BRDF samples (which are counted multiple times, once for each light).
  //
  unsigned total_samples = 0;

  // Generate samples for each light.
  //
  for (unsigned lnum = 0; lnum < num_lights; lnum++)
    {
      // Number of samples to take for this light.
      //
      LightParams &lparams = light_params[lnum];

      unsigned prev_samples_size = samples.size ();

      if (lparams.num_samples != 0)
	{
	  // Generate the samples; the sample method may generate a different
	  // number of samples than we requested, so we update our count.
	  //
	  lparams.num_samples
	    = lights[lnum]->gen_samples (isec, lparams.num_samples, samples);

	  total_samples += lparams.num_samples;
	}

      // Count the physical number of samples added.  The difference
      // between the `num_samples' and `num_real_samples' fields is
      // accounted for by "virtual samples" with a value of zero.
      //
      lparams.num_real_samples = samples.size() - prev_samples_size;

      if (! lparams.is_point_light)
	total_samples += num_brdf_samples;
    }

  if (total_samples == 0)
    return;			// whoops

  // Sample range of BRDF samples in SAMPLES.
  //
  const IllumSampleVec::iterator bs_beg = samples.begin ();
  const IllumSampleVec::iterator bs_end = bs_beg + num_real_brdf_samples;

  // Beginning of next light's samples in SAMPLES.
  //
  IllumSampleVec::iterator ls_beg = bs_end;

  // Match BRDF samples to lights.
  //
  for (unsigned lnum = 0; lnum < num_lights; lnum++)
    {
      const LightParams &lparams = light_params[lnum];

      if (lparams.num_samples + num_brdf_samples != 0)
	{
	  // See if any BRDF samples hit this light
	  //
	  lights[lnum]->filter_samples (isec, bs_beg, bs_end);

	  // Final processing of light samples.
	  //
	  if (lparams.num_real_samples != 0)
	    {
	      // End of this light's samples.
	      //
	      const IllumSampleVec::iterator ls_end
		= ls_beg + lparams.num_real_samples;

	      // Calculate BRDF response for light samples from this light.
	      //
	      isec.brdf.filter_samples (isec, ls_beg, ls_end);

	      // Amount each sample counts in the final overall illumination
	      // equation (including all samples from all lights).
	      //
	      float light_sample_weight = lparams.weight / lparams.num_samples;

	      // We handle point lights specially, as they don't use any BRDF
	      // samples, and can't use multiple importance sampling.
	      //
	      if (lparams.is_point_light)
		//
		// Point light
		{
		  // Calculate the final sample values for light samples.
		  //
		  for (IllumSampleVec::iterator s = ls_beg; s != ls_end; ++s)
		    if (! s->invalid)
		      s->val *= s->refl * light_sample_weight;
		}
	      else
		//
		// Area light
		{
		  // Calculate BRDF response for light samples from this
		  // light, and then calculate the final sample value
		  // from the light and BRDF factors.
		  //
		  for (IllumSampleVec::iterator s = ls_beg; s != ls_end; ++s)
		    if (! s->invalid)
		      {
			// Weight to adjust for multiple importance sampling
			//
			float mis_weight
			  = mis_sample_weight (s->light_pdf,
					       lparams.num_samples,
					       s->brdf_pdf, num_brdf_samples);

#if MIS_VERBOSE_DEBUG
			std::cout << "light sample " << s - ls_beg << std::endl
				  << "   orig_val = " << s->val << std::endl
				  << "   weight(lpdf = " << s->light_pdf
				  << ", lnum = " << lparams.num_samples
				  << ", bpdf = " << s->brdf_pdf
				  << ", bnum = " << num_brdf_samples
				  << ") = " << mis_weight << std::endl
				  << "   sample_weight = " << light_sample_weight
				  << " (" << lparams.weight
				  << " /  " << lparams.num_samples << ")"
				  << "   scale = "
				  << (mis_weight * light_sample_weight / s->light_pdf)
				  << std::endl
				  << "   refl = " << s->refl
				  << std::endl;
#endif
			s->val *= (s->refl * mis_weight * light_sample_weight
				   / s->light_pdf);
#if MIS_VERBOSE_DEBUG
			std::cout << "   new_val = " << s->val << std::endl
				  << std::endl;
#endif
		      }
		}

	      // Update LS_BEG for next light.
	      //
	      ls_beg = ls_end;
	    }

	  // Now do BRDF samples.  The call to `filter_samples' above will have
	  // filled in the appropriate light-related fields for every BRDF
	  // sample which intersected the light _if_ this light is closer than
	  // any other previous light that intersected that sample.  We can
	  // tell which light the current light fields are from by looking at
	  // each sample's `light' field.
	  //
	  // We simply ignore BRDF samples which don't hit, as they will
	  // contribute nothing to this light's terms (so each BRDF sample only
	  // contributes to illumination from a single light, even though it's
	  // "counted" for all lights).
	  //
#if MIS_VERBOSE_DEBUG
	  std::cout << "num_brdf_samples = " << num_brdf_samples << std::endl;
	  std::cout << "is_point_light = " << lparams.is_point_light << std::endl;
#endif
	  if (num_brdf_samples != 0 && !lparams.is_point_light)
	    {
	      float brdf_sample_weight = lparams.weight / num_brdf_samples;
#if MIS_VERBOSE_DEBUG
	      std::cout << "brdf_sample_weight = " << brdf_sample_weight << std::endl;
#endif

	      for (IllumSampleVec::iterator s = bs_beg; s != bs_end; ++s)
		if (!s->invalid && s->light == lights[lnum])
		  {
		    // Weight to adjust for multiple importance sampling
		    //
		    float mis_weight
		      = mis_sample_weight (s->brdf_pdf, num_brdf_samples,
					   s->light_pdf, lparams.num_samples);

#if MIS_VERBOSE_DEBUG
		    std::cout << "brdf sample " << s - bs_beg << std::endl
			      << "   orig_val = " << s->val << std::endl
			      << "   weight(bpdf = " << s->brdf_pdf
			      << ", bnum = " << num_brdf_samples
			      << ", lpdf = " << s->light_pdf
			      << ", lnum = " << lparams.num_samples
			      << ") = " << mis_weight << std::endl
			      << "   sample_weight = " << brdf_sample_weight
			      << " (" << lparams.weight
			      << " /  " << num_brdf_samples << ")"
			      << "   scale = "
			      << (mis_weight * brdf_sample_weight / s->brdf_pdf)
			      << std::endl
			      << "   refl = " << s->refl
			      << std::endl;
#endif
		    s->val *= s->refl * mis_weight * brdf_sample_weight / s->brdf_pdf;
#if MIS_VERBOSE_DEBUG
		    std::cout << "   new_val = " << s->val << std::endl
			      << std::endl;
#endif
		  }
	    }
	}
    }

#if MIS_VERBOSE_DEBUG
  for (IllumSampleVec::iterator s = samples.begin (); s != samples.end(); ++s)
    {
      std::cout << "dir = " << s->dir
		<< ", val = " << s->val
		<< ", refl = " << s->refl
		<< ", light_pdf = " << s->light_pdf
		<< ", brdf_pdf = " << s->brdf_pdf
		<< ", dist = " << s->dist
		<< ", light = " << (s->light ? s->light->num : -1)
		<< ", specular = " << s->specular
		<< ", invalid = " << s->invalid
		<< std::endl;
    }
#endif
}


// arch-tag: d7180220-e119-460b-8ae3-42a6426078e0
