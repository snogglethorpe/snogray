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
RecursIllum::lo (const Intersect &isec,
		 const IllumSampleVec::iterator &brdf_samples_beg,
		 const IllumSampleVec::iterator &brdf_samples_end,
		 unsigned num_brdf_samples,
		 const IllumMgr &illum_mgr)
  const
{
  float brdf_sample_weight = num_brdf_samples ? 1.f / num_brdf_samples : 1.f;

  const Medium *refr_medium = 0;
  bool calculated_refr_medium = false;
  unsigned depth = isec.trace.depth ();

  bool use_rr
    = ((isec.trace.complexity >= isec.context.params.max_brdf_samples)
       || depth > 10);

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

	    // Calculate the type of the new trace segment, and its medium.
	    //
	    Trace::Type subtrace_type;
	    const Medium *new_medium;
	    if (s->flags & IllumSample::REFLECTIVE)
	      {
		subtrace_type = Trace::REFLECTION;
		new_medium = &isec.trace.medium; // reflection, new same as old
	      }
	    else
	      {
		// Must be transmissive

		assert (s->flags & IllumSample::TRANSMISSIVE,
			"RecursIllum::lo -- sample has no direction");

		subtrace_type
		  = isec.back ? Trace::REFRACTION_OUT : Trace::REFRACTION_IN;

		if (! calculated_refr_medium)
		  {
		    if (isec.back)
		      refr_medium = &isec.trace.enclosing_medium ();
		    else
		      {
			refr_medium = isec.material->medium ();
			if (! refr_medium)
			  refr_medium = &isec.trace.medium;
		      }

		    calculated_refr_medium = true;
		  }

		new_medium = refr_medium;
	      }
	    assert (new_medium, "RecursIllum::lo -- zero medium");

	    Trace sub_trace (subtrace_type, s->isec_info->ray, *new_medium,
			     branch_factor, isec.trace);

	    val = illum_mgr.li (s->isec_info, sub_trace);

	    val *= rr_adj;
	  }
	else
	  val = isec.context.scene.background (isec.normal_frame.from (s->dir));

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
