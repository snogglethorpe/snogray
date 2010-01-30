// illum-mgr.cc -- Sample-based manager for illuminators
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
#include "media.h"
#include "brdf.h"

#include "illum-mgr.h"


using namespace snogray;


IllumMgr::IllumHandler::~IllumHandler ()
{
  delete illum;
}



// Add the illuminator ILLUM to the list of illuminators.  It will be
// applied only to BRDF samples who have some flags in MASK which are
// non-zero after being, xored with INVERT.  As a special case, if MASK
// is zero, the illuminator will always be applied.
//
void
IllumMgr::add_illum (Illum *illum, unsigned mask, unsigned invert)
{
  illum_handlers.push_back (IllumHandler (illum, mask, invert));

  // Update ANY_USES with ILLUM's uses flags.
  //
  any_uses |= illum->uses;

  // Record implicit uses-flags in ANY_USES too (even if ILLUM doesn't
  // use the info directly, it's needed to supply the info needed for
  // masking sample flags in Illum::lo).
  //
  if (mask & IllumSample::DIRECT)
    any_uses |= Illum::USES_DIRECT_INFO;
}



// Partition samples in the between BEG and END, so that all "matching"
// samples come before all "non-matching" samples.  The partition point
// (first non-matching sample) is returned.
//
IllumSampleVec::iterator
IllumMgr::IllumHandler::partition_samples (IllumSampleVec::iterator beg,
					   IllumSampleVec::iterator end)
  const
{
  unsigned mask = sample_flags_mask, invert = sample_flags_invert;

  mask |= invert;

  if (mask == 0)
    return end;

  while (beg != end)
    if ((beg->flags & mask) ^ invert)
      ++beg;
    else
      std::iter_swap (beg, --end);

  return beg;
}


// IllumMgr::lo

// Return the color emitted from the ray-intersection ISEC.
// "Lo" means "Light outgoing".
//
Color
IllumMgr::Lo (const Intersect &isec, unsigned depth, float complexity) const
{
  RenderContext &context = isec.context;
  const Scene &scene = context.scene;

  context.stats.illum_calls++;

  // Accumulated radiance.
  //
  Color radiance = 0;

  radiance += isec.material->le (isec);

  if (! isec.brdf)
    return radiance;

  // STL allocator for allocating samples.
  //
  IllumSampleVecAlloc samp_alloc (isec.mempool ());

  // Vector of BRDF samples.
  //
  IllumSampleVec brdf_samples (samp_alloc);

  // Number of non-specular BRDF samples taken (this may be different
  // than the length of BRDF_SAMPLES, if some samples were omitted
  // because they had a zero value, or if some samples are specular).
  // This is initially zero, as no BRDF samples have been generated yet.
  //
  unsigned num_brdf_samples = 0;

  // If true, we prune "direct" samples by masking them against lights
  // if full intersection info isn't needed.  For small numbers of
  // lights this is cheaper than doing a full scene intersection test,
  // but obviously it can't be done if there's full-sphere environmental
  // light (because _every_ ray intersects that, although many are
  // shadowed by some intervening object).
  //
  bool prune_direct_samples_against_lights = ! scene.bg_set;

  // Beginning of BRDF sample region that will be passed to the
  // illuminator (this will be updated below when BRDF samples are
  // actually generated).
  //
  IllumSampleVec::iterator bs_beg = brdf_samples.begin ();

  // Iniitally the union of all Illum::Uses flags for all illuminators;
  // as various data is generated, these bits will be turned off (this
  // is only done for "one-time" data).
  //
  unsigned pending_uses = any_uses;

  // Iterate over illuminators, passing them whichever BRDF samples
  // mask the flags they specify.
  //
  for (std::list<IllumHandler>::const_iterator ih = illum_handlers.begin ();
       ih != illum_handlers.end (); ++ih)
    {
      Illum *illum = ih->illum;
      unsigned uses = illum->uses;

      //
      // Note: the following code is a bit convoluted, mostly trying to
      // avoid doing intersection testing wherever possible.
      // Intersection testing is so incredibly expensive (often 90% of
      // runtime) that we want to do it only for those samples which
      // really require it.
      //

      // End of BRDF sample region that will be passed to the
      // illuminator (this will be updated below to its final value).
      //
      // Illuminators that don't use BRDF samples will be passed an
      // empty region.
      //
      IllumSampleVec::iterator bs_end = bs_beg;

      if (uses & Illum::USES_BRDF_SAMPLES)
	{
	  // Turn on implicit uses-flags (even if ILLUM doesn't use the
	  // info directly, it's needed to supply the info needed for
	  // masking sample flags).
	  //
	  if (ih->sample_flags_mask & IllumSample::DIRECT)
	    uses |= Illum::USES_DIRECT_INFO;

	  // An iterator referring to the end of the entire vector of
	  // brdf samples, until sample partitioning is done, at which
	  // point it is moved to the end of the current sample region.
	  //
	  IllumSampleVec::iterator end = brdf_samples.end ();

	  // Generate BRDF samples.
	  //
	  if (pending_uses & Illum::USES_BRDF_SAMPLES)
	    {
	      unsigned local_brdf_samples = context.params.num_brdf_samples;
	      unsigned global_brdf_samples = context.params.max_brdf_samples;

	      if (global_brdf_samples != 0)
		{
		  unsigned desired_brdf_samples
		    = clamp (unsigned (global_brdf_samples / complexity + 0.5f),
			     1u, local_brdf_samples);

		  num_brdf_samples
		    = isec.brdf->gen_samples (desired_brdf_samples,
					      brdf_samples);

		  // Update iterators pointing into the modified vector.
		  //
		  end = brdf_samples.end ();
		  bs_beg = bs_end = brdf_samples.begin ();

		  context.stats.illum_samples += brdf_samples.size ();
		}

	      pending_uses &= ~Illum::USES_BRDF_SAMPLES;
	    }

	  // Normally we partition the BRDF sample vector (based on the
	  // sample flags) just before invoking the illuminator.
	  //
	  // However in some cases we want to do it before then, in
	  // which case this flag get set to true to suppress the usual
	  // partitioning.
	  //
	  bool partitioned_early = false;

	  // If ILLUM wants direct samples, but doesn't need full
	  // intersection info, calculate shadowing info using a cheaper
	  // method.
	  //
	  // This is done for all samples, so there's a risk that we
	  // will end up calculating intersection info twice for some
	  // (once here, for all remaining samples, and later for other
	  // illuminators that need full intersection info).  This risk
	  // can be mitigated to some degree by the ordering of the
	  // illuminators.
	  //
	  if (((uses & pending_uses) & Illum::USES_DIRECT_INFO)
	      && ! (uses & Illum::USES_INTERSECT_INFO))
	    {
	      // If PRUNE_DIRECT_SAMPLES_AGAINST_LIGHTS is true, then
	      // first try to prune the samples by rejecting those that
	      // can't possibly hit any light.
	      //
	      if (prune_direct_samples_against_lights)
		{
		  // Filter all remaining samples through each light's
		  // Light::filter_samples method, which will fill in
		  // the samples' light-related fields.
		  //
		  const std::vector<const Light *> &lights = scene.lights;
		  for (std::vector<const Light *>::const_iterator li
			 = lights.begin ();
		       li != lights.end (); ++li)
		    (*li)->filter_samples (isec, bs_beg, end);

		  // Make sure we don't try to calculate light info again.
		  //
		  pending_uses &= ~Illum::USES_LIGHT_INFO;

		  // Also avoid re-recalculating it below for this illum.
		  //
		  uses &= ~Illum::USES_LIGHT_INFO;
		}

	      // Now do shadow-testing using the cheapest intersection
	      // test, possibly ignoring any samples that failed the
	      // "light test" above.
	      //
	      dist_t min_dist = context.params.min_trace;
	      for (IllumSampleVec::iterator s = bs_beg; s != end; ++s)
		if (!prune_direct_samples_against_lights
		    || (s->light && s->light_val > 0))
		  {
		    dist_t max_dist
		      = (s->light_dist
			 ? s->light_dist - min_dist
			 : scene.horizon);

		    ShadowRay ray (isec, isec.normal_frame.from (s->dir),
				   min_dist, max_dist);

		    // XXX HACK: the "no_self_shadowing" flag in ISEC can be
		    // incorrect for refractive samples, so temporarily turn
		    // it off when shadow-testing them.  XXX
		    //
		    bool no_self_shadowing = isec.no_self_shadowing;
		    if (s->flags & IllumSample::TRANSMISSIVE)
		      const_cast<Intersect &>(isec).no_self_shadowing
			= false;

		    if (! scene.intersects (ray, context))
		      s->flags |= IllumSample::DIRECT;

		    /// XXX hack continued XXX
		    //
		    const_cast<Intersect &>(isec).no_self_shadowing
		      = no_self_shadowing;
		  }

	      // Make sure we don't try to calculate shadowing info again.
	      //
	      pending_uses &= ~Illum::USES_DIRECT_INFO;
	    }

	  // Do intersection testing if necessary.
	  //
	  // In certain cases (namely in the PARTITIONED_EARLY false
	  // case) we calculate intersection information for all
	  // remaining samples; if that's already been done, we don't
	  // need to do so again.  In such a case, the
	  // Illum::USES_INTERSECT_INFO flag will have been cleared in
	  // PENDING_USES -- thus the slightly strange expression
	  // "pending & uses & FLAGS...", which tests both that the info
	  // hasn't been calculated yet, and also is desired by this
	  // particular illuminator.
	  //
	  if ((uses & pending_uses)
	      & (Illum::USES_INTERSECT_INFO | Illum::USES_DIRECT_INFO))
	    {
	      // If doing intersection testing, and the illuminator
	      // _doesn't_ depend on the IllumSample::DIRECT flag
	      // (meaning it uses samples regardless of whether they are
	      // shadowed or not), we want to do the partitioning before
	      // doing intersection-testing (instead of in the usual
	      // place just before the illuminator call), so we can
	      // avoid doing expensive intersection testing for samples
	      // we know this illuminator won't use.
	      //
	      if (! (ih->sample_flags_mask & IllumSample::DIRECT))
		{
		  bs_end = end = ih->partition_samples (bs_beg, end);
		  partitioned_early = true;
		}
	      else
		{
		  // If we didn't do early partitioning, we'll end up
		  // calculating intersection info for all remaining
		  // samples.  Record that fact so we can avoid doing so
		  // again.  Since we also calculate direct/non-direct
		  // info, we also turn of the Illum::USES_DIRECT_INFO
		  // flag.
		  //
		  pending_uses
		    &= ~(Illum::USES_INTERSECT_INFO | Illum::USES_DIRECT_INFO);
		}

	      dist_t min_dist = context.params.min_trace;

	      for (IllumSampleVec::iterator s = bs_beg; s != end; ++s)
		{
		  dist_t max_dist
		    = (s->light_dist
		       ? s->light_dist - min_dist
		       : scene.horizon);

		  Ray ray (isec.normal_frame.origin,
			   isec.normal_frame.from (s->dir),
			   min_dist, max_dist);

		  s->isec_info = scene.intersect (ray, context);

		  if (s->isec_info)
		    //
		    // This sample hit something, so see if it's a light.
		    {
		      if (s->isec_info->material()->emits_light ())
			s->flags |= IllumSample::DIRECT;
		    }
		  else if (scene.bg_set)
		    //
		    // If there's a scene background, all rays that
		    // don't hit anything implicitly hit a light.
		    {
		      s->flags |= IllumSample::DIRECT;
		    }
		}
	    }

	  // If we didn't do early partitioning, partition the samples now.
	  //
	  if (! partitioned_early)
	    bs_end = ih->partition_samples (bs_beg, end);

	  // If this illuminator wants sample light info, and we didn't
	  // calculate that above.
	  //
	  if (uses & Illum::USES_LIGHT_INFO)
	    {
	      const std::vector<const Light *> &lights = scene.lights;
	      for (std::vector<const Light *>::const_iterator li
		     = lights.begin ();
		   li != lights.end (); ++li)
		(*li)->filter_samples (isec, bs_beg, bs_end);
	    }
	}

      // Call the illuminator with the BRDF samples it wanted.
      //
      radiance += ih->illum->Lo (isec, bs_beg, bs_end, num_brdf_samples,
				 *this, depth, complexity);

      // Advance BS_BEG for the next illuminator.
      //
      bs_beg = bs_end;
    }

  return radiance;
}
