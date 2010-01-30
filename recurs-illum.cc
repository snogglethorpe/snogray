// recurs-illum.cc -- Illuminator using recursive tracing.
//
//  Copyright (C) 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "illum-mgr.h"
#include "snogassert.h"

#include "recurs-illum.h"


using namespace snogray;


// Return outgoing radiance for this illuminator.  The BRDF samples
// between BRDF_SAMPLES_BEG and BRDF_SAMPLES_END are matched to this
// illuminator.  NUM_BRDF_SAMPLES is the total number of non-specular
// BRDF samples generated (even those not passed to this illuminator).
//
// ILLUM_MGR can be used for recursively calculating illumination.
//
Color
RecursIllum::Lo (const Intersect &isec,
		 const IllumSampleVec::iterator &brdf_samples_beg,
		 const IllumSampleVec::iterator &brdf_samples_end,
		 unsigned num_brdf_samples,
		 const IllumMgr &illum_mgr, float complexity)
  const
{
  RenderContext &context = isec.context;

  float brdf_sample_weight = num_brdf_samples ? 1.f / num_brdf_samples : 1.f;

  unsigned depth = isec.trace.depth ();

  bool use_rr = ((complexity >= context.params.max_brdf_samples) || depth > 10);

  float branch_factor = 0;
  for (IllumSampleVec::iterator s = brdf_samples_beg;
       s != brdf_samples_end; ++s)
    if (s->brdf_val > Eps)
      branch_factor++;

  Color radiance = 0;

  for (IllumSampleVec::iterator s = brdf_samples_beg;
       s != brdf_samples_end; ++s)
    if (s->brdf_val > Eps)
      {
	Color val;

	if (s->isec_info)
	  {
	    float rr_adj = 1;

	    if (use_rr)
	      {
		// Use russian-roulette

		rr_adj = depth;

		rr_adj /= s->brdf_val.intensity ();
		if (! (s->flags & IllumSample::SPECULAR))
		  rr_adj *= s->brdf_pdf;

		if (rr_adj < 1)
		  rr_adj = 1;
		else if (random (rr_adj) > 1)
		  continue;
	      }

	    // Get more intersection info.
	    //
	    Trace sub_trace (isec, s->isec_info->ray,
			     (s->flags & IllumSample::TRANSMISSIVE),
			     branch_factor);
	    Intersect isec = s->isec_info->make_intersect (sub_trace, context);

	    // Calculate the appearance of the point on the surface we hit
	    //
	    val = illum_mgr.Lo (isec, complexity * branch_factor);

	    // If we are looking through something other than air,
	    // attentuate the surface appearance due to transmission
	    // through the current medium.
	    //
	    val *= context.volume_integ->transmittance (s->isec_info->ray,
							sub_trace.medium);

	    val *= abs (isec.cos_n (s->dir));

	    val *= rr_adj;
	  }
	else
	  val = context.scene.background (isec.normal_frame.from (s->dir));

	//
	// The multiplications below are a potential point of overflow.  When
	// a that happens, VAL becomes an IEEE infinity.  However, perhaps it
	// would be better to clamp VAL to FLT_MAX instead of letting the
	// infinity propagate?
	//

	val *= s->brdf_val;

	if (! (s->flags & IllumSample::SPECULAR))
	  val *= brdf_sample_weight / s->brdf_pdf;

	radiance += val;
      }

  return radiance;
}
