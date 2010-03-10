// photon-integ.cc -- Photon-mapping surface integrator
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

#include <iostream>

#include "snogassert.h"
#include "snogmath.h"
#include "scene.h"
#include "bsdf.h"
#include "media.h"
#include "progress.h"
#include "string-funs.h"
#include "radical-inverse.h"
#include "mis-sample-weight.h"

#include "photon-integ.h"


using namespace snogray;



// Constructors etc

// For arguments, a out-of-band value used to detect unspecified
// parameters.
//
static const unsigned UNSPEC_UINT = 99999;

PhotonInteg::GlobalState::GlobalState (const GlobalRenderState &rstate,
				       const ValTable &params)
  : RecursiveInteg::GlobalState (rstate),
    num_search_photons (params.get_uint ("num", 50)),
    photon_search_radius (params.get_float ("radius", 0.1)),
    caustic_scale (0), direct_scale (0), indirect_scale (0),
    marker_radius_sq (params.get_float ("marker-radius", 0)),
    direct_illum (
      params.get_uint ("direct-samples,dir-samples,dir-samps",
		       rstate.params.get_uint ("light-samples", 16))),
    use_direct_illum (params.get_bool ("direct-illum,dir-illum", true)),
    num_fgather_samples (
      params.get_uint ("final-gather-samples,fg-samples,fg-samps",
		       UNSPEC_UINT)),
    num_fgather_photon_samples (
      params.get_uint ("final-gather-photon-samples"
		       ",fg-photon-samples,fg-photon-samps",
		       UNSPEC_UINT)),
    num_fgather_bsdf_samples (
      params.get_uint ("final-gather-bsdf-samples"
		       ",fg-bsdf-samples,fg-bsdf-samps",
		       UNSPEC_UINT))
{
  unsigned num_caustic = params.get_uint ("caustic", 50000);
  unsigned num_direct = params.get_uint ("direct,dir", 500000);
  unsigned num_indirect = params.get_uint ("indirect,indir", 500000);

  // A convenient boolean toggle for final gathering.
  //
  if (params.get_bool ("final-gather,fg", true))
    {
      // Allocate various types of final-gather samples for MIS.
      // We only support a few combinations of user parameters:
      //
      //  * only fg-samps specified:  divide fg-samps between photon and bsdf
      //
      //  * both fg-photon-samps and fg-bsdf-samps specified, but
      //    fg-samps not specified:  set fg-samps to the sum of photon
      //    and bsdf samples
      //
      //  * all specified:  leave as given
      //
      if (num_fgather_samples == UNSPEC_UINT
	  && num_fgather_bsdf_samples == UNSPEC_UINT
	  && num_fgather_photon_samples == UNSPEC_UINT)
	{
	  // Nothing specified, use defaults

	  num_fgather_samples = 16;
	  num_fgather_bsdf_samples = 8;
	  num_fgather_photon_samples = 8;
	}
      else if (num_fgather_samples == UNSPEC_UINT)
	{
	  // Total number not specified; set it to the sum of the two types
	  // (if only one of them was actually given, we default it to zero).

	  if (num_fgather_photon_samples == UNSPEC_UINT)
	    num_fgather_photon_samples = 0;
	  if (num_fgather_bsdf_samples == UNSPEC_UINT)
	    num_fgather_bsdf_samples = 0;

	  num_fgather_samples
	    = num_fgather_bsdf_samples + num_fgather_photon_samples;
	}
      else if (num_fgather_bsdf_samples == UNSPEC_UINT
	       || num_fgather_photon_samples == UNSPEC_UINT)
	{
	  // Total was specified, but one or both of the two types wasn't.
	  // Divide it amongst the two.

	  if (num_fgather_bsdf_samples == UNSPEC_UINT
	      && num_fgather_photon_samples == UNSPEC_UINT)
	    {
	      // In the special case of one sample only, use randomization.
	      //
	      if (num_fgather_samples == 1)
		num_fgather_bsdf_samples = num_fgather_photon_samples = 0;
	      else
		{
		  num_fgather_photon_samples = num_fgather_samples / 2;
		  num_fgather_bsdf_samples = (num_fgather_samples + 1) / 2;
		}
	    }
	  else if (num_fgather_bsdf_samples == UNSPEC_UINT)
	    num_fgather_photon_samples
	      = num_fgather_samples - num_fgather_bsdf_samples;
	  else
	    num_fgather_bsdf_samples
	      = num_fgather_samples - num_fgather_photon_samples;
	}
      // otherwise, all must have specified, so leave as-is.
    }
  else
    {
      // No final gathering.

      num_fgather_samples = 0;
      num_fgather_bsdf_samples = 0;
      num_fgather_photon_samples = 0;
    }

  // If using the usual direct lighting calculation, and not doing final
  // gathering, there's no need for direct photons.
  //
  if (use_direct_illum && num_fgather_samples == 0)
    num_direct = 0;

  marker_radius_sq *= marker_radius_sq;

  generate_photons (num_caustic, num_direct, num_indirect);

  std::cout << "* photon-integ: ";
  if (use_direct_illum)
    std::cout << direct_illum.num_light_samples << " direct sample"
	      << (direct_illum.num_light_samples == 1 ? "" : "s");
  else
    std::cout << "no direct illum";
  std::cout << ", ";
  if (num_fgather_samples != 0)
    std::cout << num_fgather_samples << " final-gather sample"
	      << (num_fgather_samples == 1 ? "" : "s")
	      << " (" << num_fgather_photon_samples << " photon, "
	      << num_fgather_bsdf_samples << " BSDF)";
  else
    std::cout << "no final-gathering";
  std::cout << std::endl;
}

// Integrator state for rendering a group of related samples.
//
PhotonInteg::PhotonInteg (RenderContext &context, GlobalState &global_state)
  : RecursiveInteg (context), global (global_state),
    // photon_dir_hist (64, 64), photon_dir_dist (64, 64),
    photon_dir_hist (8, 8), photon_dir_dist (8, 8),
    direct_illum (context, global_state.direct_illum),
    fgather_bsdf_chan (
      context.samples.add_channel<UV> (global.num_fgather_bsdf_samples)),
    fgather_bsdf_layer_chan (
      context.samples.add_channel<float> (global.num_fgather_bsdf_samples)),
    fgather_photon_chan (
      context.samples.add_channel<UV> (global.num_fgather_photon_samples))
{
}

// Return a new integrator, allocated in context.
//
SurfaceInteg *
PhotonInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new PhotonInteg (context, *this);
}


// PhotonInteg::GlobalState::generate_photons

// Generate the specified number of photons and add them to our photon-maps.
//
void
PhotonInteg::GlobalState::generate_photons (unsigned num_caustic,
					    unsigned num_direct,
					    unsigned num_indirect)
{
  RenderContext context (global_render_state);
  Media surrounding_media (context.default_medium);

  std::vector<Photon> caustic_photons;
  std::vector<Photon> direct_photons;
  std::vector<Photon> indirect_photons;

  const std::vector<Light *> &lights = context.scene.lights;

  if (lights.size () == 0)
    return;			// no lights, so no point

  Progress prog (std::cout, "* photon-integ: shooting photons...",
		 0, num_caustic + num_direct + num_indirect);

  prog.start ();

  bool caustic_done = (num_caustic == 0);
  bool direct_done = (num_direct == 0);
  bool indirect_done = (num_indirect == 0);

  // Number of paths tried for each type of photon.
  //
  unsigned num_caustic_paths = 0, num_direct_paths = 0, num_indirect_paths = 0;

  for (unsigned path_num = 0;
       !caustic_done || !direct_done || !indirect_done;
       path_num++) 
    {
      prog.update (caustic_photons.size()
		   + direct_photons.size()
		   + indirect_photons.size());

      // Randomly choose a light.
      //
      unsigned light_num = radical_inverse (path_num, 11) * lights.size ();
      const Light *light = lights[light_num];

      // Sample the light.
      //
      UV pos_param (radical_inverse (path_num, 2),
		    radical_inverse (path_num, 3));
      UV dir_param (radical_inverse (path_num, 5),
		    radical_inverse (path_num, 7));
      Light::FreeSample samp = light->sample (pos_param, dir_param);
      
      if (samp.val == 0 || samp.pdf == 0)
	continue;

      // Update the number of paths generated.  Every light sample is a
      // potential photon path for all photon types that haven't
      // finished yet (we do all types in parallel).
      //
      if (! caustic_done)
	num_caustic_paths++;
      if (! direct_done)
	num_direct_paths++;
      if (! indirect_done)
	num_indirect_paths++;

      // The logical-or of all the Bsdf::SURFACE_CLASS flags we
      // encounter in while bouncing around surfaces in the scene.  It
      // starts out as zero, meaning we've just left the light.
      //
      unsigned bsdf_history = 0;

      // Stack of Media objects at current location.
      //
      const Media *innermost_media = &surrounding_media;

      // The current postion / direction / power of the photon we're
      // shooting.
      //
      Pos pos = samp.pos;
      Vec dir = samp.dir;
      Color power = samp.val * float (lights.size ()) / samp.pdf;

      // We keep shooting the photon PH into the scene, and follow it as
      // it bounces off surfaces.  The loop is terminated if PH fails to
      // hit anything, hits a non-scatting (matte black) surface, or is
      // terminated by russian-roulette.
      //
      for (unsigned path_len = 0; ; path_len++)
	{
	  Ray ray (pos, dir, context.params.min_trace, context.scene.horizon);

	  // See if RAY hits something.
	  //
	  const Surface::IsecInfo *isec_info
	    = context.scene.intersect (ray, context);

	  // Photon escaped, give up.
	  //
	  if (! isec_info)
	    break;

	  // Top of current media stack.
	  //
	  const Media &media = *innermost_media;

	  // Get more information about the intersection.
	  //
	  Intersect isec = isec_info->make_intersect (media, context);

	  // If there's no BSDF, give up (this surface cannot scatter light).
	  //
	  if (! isec.bsdf)
	    break;

	  // Reduce the photon's power to reflect any media attentuation.
	  //
	  power *= context.volume_integ->transmittance (ray, media.medium);

	  // Now maybe deposit a photon at this location.  We don't do
	  // this for purely specular surfaces.
	  //
	  if (isec.bsdf->supports (Bsdf::ALL & ~Bsdf::SPECULAR))
	    {
	      // The photon we're going to store.  Note that the
	      // direction is reversed, as the photon's direction points
	      // to where it _came_ from.
	      //
	      Photon ph (isec.normal_frame.origin, -dir, power);

	      // Choose which photon-map to put PH in; if the map we put
	      // it into ends up being full (after we store PH),
	      // remember the number of paths it took to get to this
	      // point.
	      //
	      if (bsdf_history == 0)
		// direct; path-type:  L(D|G)
		{
		  if (! direct_done)
		    {
		      // Deposit a direct photon.

		      direct_photons.push_back (ph);
		      direct_done = (direct_photons.size() == num_direct);
		    }
		}
	      else if (num_caustic != 0
		       && ! (bsdf_history & Bsdf::SURFACE_CLASS & ~Bsdf::SPECULAR))
		// caustic; path-type:  L(S)+(D|G)
		{
		  if (! caustic_done)
		    {
		      // Deposit a caustic photon.
		      //
		      caustic_photons.push_back (ph);
		      caustic_done = (caustic_photons.size() == num_caustic);
		    }
		}
	      else
		// indirect; path-type:  L(D|G|S)*(D|G)(D|G|S)*
		{
		  if (! indirect_done)
		    {
		      // Deposit a caustic photon.
		      //
		      indirect_photons.push_back (ph);
		      indirect_done = (indirect_photons.size() == num_indirect);
		    }
		}
	    }

	  // Now sample the BSDF to continue this photon's path.
	  //
	  UV bsdf_samp_param
	    = (path_len == 0
	       ? UV (radical_inverse (path_num, 13),
		     radical_inverse (path_num, 17))
	       : UV (context.random (), context.random ()));
	  Bsdf::Sample bsdf_samp = isec.bsdf->sample (bsdf_samp_param);

	  if (bsdf_samp.val == 0 || bsdf_samp.pdf == 0)
	    break;

	  // Maybe terminate the path using russian-roulette.
	  //
	  if (path_len > 3)
	    {
	      float rr_terminate_probability = 0.5f;
	      float russian_roulette = context.random ();
	      if (russian_roulette < rr_terminate_probability)
		break;
	      else
		power /= rr_terminate_probability;
	    }

	  // Update the position/direction/power of the photon for the
	  // next segment.
	  //
	  pos = isec.normal_frame.origin;
	  dir = isec.normal_frame.from (bsdf_samp.dir);
	  power *= bsdf_samp.val * abs (isec.cos_n (bsdf_samp.dir)) / bsdf_samp.pdf;

	  // Remember the type of reflection/refraction in our history.
	  //
	  bsdf_history |= bsdf_samp.flags;

	  // If we just followed a refractive (transmissive) sample, we need
	  // to update our stack of Media entries:  entering a refractive
	  // object pushes a new Media, existing one pops the top one.
	  //
	  if (bsdf_samp.flags & Bsdf::TRANSMISSIVE)
	    Media::update_stack_for_transmission (innermost_media, isec);
	}

      context.mempool.reset ();

#if 0
      // Detect degenerate scene conditions which are preventing photons
      // from being generated, and disable the affected categories.
      //
      if (caustic_photons.empty() && path_num > num_caustic * 10)
	caustic_done = true;
      if (direct_photons.empty() && path_num > num_direct * 10)
	direct_done = true;
      if (indirect_photons.empty() && path_num > num_indirect * 10)
	indirect_done = true;
#endif

      if (path_num > 1e8)
	break;
    }

  prog.end ();

  caustic_photon_map.set_photons (caustic_photons);
  direct_photon_map.set_photons (direct_photons);
  indirect_photon_map.set_photons (indirect_photons);

  if (num_caustic_paths > 0)
    caustic_scale = 1 / float (num_caustic_paths);
  if (num_direct_paths > 0)
    direct_scale = 1 / float (num_direct_paths);
  if (num_indirect_paths > 0)
    indirect_scale = 1 / float (num_indirect_paths);

  // Output information message about results.
  //
  bool some = false;
  std::cout << "* photon-integ: ";
  if (caustic_photon_map.size () != 0)
    {
      std::cout << commify (caustic_photon_map.size ()) << " caustic";
      std::cout << " (" << commify (num_caustic_paths) << " paths)";
      some = true;
    }
  if (direct_photon_map.size () != 0)
    {
      if (some)
	std::cout << ", ";
      std::cout << commify (direct_photon_map.size ()) << " direct";
      std::cout << " (" << commify (num_direct_paths) << " paths)";
      some = true;
    }
  if (indirect_photon_map.size () != 0)
    {
      if (some)
	std::cout << ", ";
      std::cout << commify (indirect_photon_map.size ()) << " indirect";
      std::cout << " (" << commify (num_indirect_paths) << " paths)";
      some = true;
    }
  if (! some)
    std::cout << "no photons generated!";
  std::cout << std::endl;
}


// PhotonInteg::Lo_photon

// Return the light emitted from ISEC by photons found nearby in
// PHOTON_MAP.  SCALE is the amount by which to scale each photon's
// radiance.  FLAGS gives the types of BSDF interaction to consider
// (by default, all).
//
Color
PhotonInteg::Lo_photon (const Intersect &isec, const PhotonMap &photon_map,
			float scale, unsigned flags)
{
  if (scale == 0)
    return 0;

  // Give up if this is a purely specular surface, or one that doesn't
  // support FLAGS.
  //
  if (! isec.bsdf->supports (flags & ~Bsdf::SPECULAR))
    return 0;

  const Pos &pos = isec.normal_frame.origin;

  found_photons.clear ();
  dist_t max_dist_sq
    = photon_map.find_photons (pos,
			       global.num_search_photons,
			       global.photon_search_radius,
			       found_photons);

  // Pre-compute values used for GAUSS_FILT in the loop.
  //
  float gauss_alpha = 1.818f, gauss_beta = 1.953f;
  float inv_gauss_denom = 1 / (1 - exp (-gauss_beta));
  float gauss_exp_scale = -gauss_beta * 0.5f / max_dist_sq;

  Color radiance = 0;

  for (std::vector<const Photon *>::iterator i = found_photons.begin();
       i != found_photons.end (); ++i)
    {
      const Photon &ph = **i;

      // Evaluate the BSDF in the photon's direction.
      //
      Vec dir = isec.normal_frame.to (ph.dir);
      Bsdf::Value bsdf_val = isec.bsdf->eval (dir, flags);

      if (bsdf_val.pdf != 0 && bsdf_val.val > 0)
	{
	  // A gaussian filter, which emphasizes photons nearer to POS,
	  // and de-emphasizes those farther away.
	  //
	  float gauss_filt
	    = (gauss_alpha
	       * (1 - ((1 - exp (gauss_exp_scale
				 * (ph.pos - pos).length_squared()))
		       * inv_gauss_denom)));

	  radiance += bsdf_val.val * ph.power * gauss_filt;
	}

      // XXXX  PBRT avoids calling Bsdf::eval more than once for
      // diffuse surfaces (since they have a constant value)....
      // worthwhile?  probably not  XXXX
    }

  radiance *= scale;
  radiance /= max_dist_sq * PIf;

  // Add photon position marker for debugging.
  //
  if (global.marker_radius_sq != 0)
    {
      for (std::vector<const Photon *>::iterator i = found_photons.begin();
	   i != found_photons.end (); ++i)
	{
	  const Photon &ph = **i;
	  dist_t dist_sq = (ph.pos - isec.normal_frame.origin).length_squared();
	  if (dist_sq < global.marker_radius_sq)
	    {
	      radiance = Color(0,1,0);
	      break;
	    }
	}
    }

  return radiance;
}



// PhotonInteg::Lo_fgather_samp

// Return a quick estimate of the outgoing radiance from ISEC which
// is due to BSDF_SAMP.  INDIR_EMISSION_SCALE is used to scale
// surface (or background) emission for recursive calls (i.e., when
// DEPTH > 0); emission is always omitted when DEPTH == 0, because
// that represents direct illumination, which be handled elsewhere.
// DEPTH is the recursion depth; it is zero for all external
// callers, and incremented during recursive calls.
//
Color
PhotonInteg::Lo_fgather_samp (const Intersect &isec, const Media &media,
			      const Bsdf::Sample &bsdf_samp,
			      const Color &indir_emission_scale, unsigned depth)
{
  Color radiance = 0;

  if (bsdf_samp.val > 0 && bsdf_samp.pdf != 0)
    {
      // Sample position and direction in world coordinates.
      //
      const Pos &pos = isec.normal_frame.origin;
      Vec dir = isec.normal_frame.from (bsdf_samp.dir);

      // Outgoing sample ray.
      //
      Ray ray (pos, dir, context.params.min_trace, context.scene.horizon);

      // See if RAY hits something.
      //
      const Surface::IsecInfo *isec_info
	= context.scene.intersect (ray, context);

      if (isec_info)
	{
	  // We hit a surface!  Do a quick radiance calculation
	  // using only photon maps.

	  Intersect samp_isec = isec_info->make_intersect (media, context);

	  if (samp_isec.bsdf)
	    {
	      Color Li
		= (Lo_photon (samp_isec,
			      global.direct_photon_map,
			      global.direct_scale)
		   + Lo_photon (samp_isec,
				global.indirect_photon_map,
				global.indirect_scale)
		   + Lo_photon (samp_isec,
				global.caustic_photon_map,
				global.caustic_scale));

	      // Adjustment to compute outgoing radiance due to
	      // BSDF_SAMP, due to incoming radiance from BSDF_SAMP.
	      //
	      Color Li_to_Lo
		= (bsdf_samp.val
		   * abs (isec.cos_n (bsdf_samp.dir))
		   / bsdf_samp.pdf);

	      // Compute outgoing light from incoming.
	      //
	      radiance += Li * Li_to_Lo;

	      // As we don't deposit photons on purely specular
	      // surfaces, the above calculation will be completely
	      // wrong for them.  To solve this, recursively handle
	      // specular surfaces.
	      //
	      unsigned spec_flags = Bsdf::SAMPLE_DIR | Bsdf::SPECULAR;
	      if (samp_isec.bsdf->supports (spec_flags) && depth < 3)
		{
		  UV samp_param (context.random(), context.random());
		  Bsdf::Sample recurs_samp
		    = samp_isec.bsdf->sample (samp_param, spec_flags);

		  radiance
		    += (Lo_fgather_samp (samp_isec, media, recurs_samp,
					 indir_emission_scale, depth + 1)
			* Li_to_Lo);
		}
	    }

	  // If DEPTH > 0, this an indirect case, so handle emission
	  // according to INDIR_EMISSION_SCALE.
	  //
	  if (depth != 0)
	    radiance
	      += samp_isec.material->Le (samp_isec) * indir_emission_scale;
	}

      else if (depth != 0)
	{
	  // We didn't hit anything, and DEPTH > 0, so this is an
	  // indirect case; include background emission according to
	  // INDIR_EMISSION_SCALE.

	  radiance += context.scene.background (dir) * indir_emission_scale;
	}
    }

  return radiance;
}


// PhotonInteg::Lo_fgather

// "Final gathering": Do a quick calculation of indirection
// illumination by sampling the BRDF, shooting another level of
// rays, and using only photon maps to calculate outgoing
// illumination from the resulting intersections.
//
// For samples that strike perfectly specular materials, recursive
// sampling is used used until a non-specular surface is hit, and
// then the photon-map is evaluated at that point; this handles
// indirect illumination due to caustics, etc.
//
// If AVOID_CAUSTICS_ON_DIFFUSE is true, then any contribution of
// caustics on diffuse surfaces is intentionally ignored (this is
// useful because such effects are usually handled via a separate
// caustics photon-pap).
//
Color
PhotonInteg::Lo_fgather (const Intersect &isec, const Media &media,
			 const SampleSet::Sample &sample,
			 bool avoid_caustics_on_diffuse)
{
  // Outgoing radiance.
  //
  Color radiance = 0;

  // Number of samples we should use for the two types of sampling
  // we use for MIS.
  //
  unsigned num_bsdf_samples = global.num_fgather_bsdf_samples;
  unsigned num_photon_samples = global.num_fgather_photon_samples;

  // Initialize variables used for photon distribution sampling.  We do
  // this even if we aren't actually doing such sampling, so that other
  // uses below will just see zeros in that case.
  //
  found_photons.clear ();
  photon_dir_hist.clear ();

  //
  // (1) Sample based on the distribution of photon directions near ISEC.
  //

  // Find nearby photons if needed, and add them to PHOTON_DIR_HIST.
  //
  if (num_photon_samples != 0)
    {
      // True if PHOTON_DIR_HIST is entirely zero, and set to false if
      // we add at least one non-zero entry.
      //
      bool all_zero_hist = true;

      // Find indirect photons near ISEC so we can sample based on their
      // distribution.
      //
      global.indirect_photon_map.find_photons (isec.normal_frame.origin,
					       global.num_search_photons,
					       global.photon_search_radius,
					       found_photons);

      // Generate a distribution from the photon directions we found.
      //
      for (std::vector<const Photon *>::iterator i = found_photons.begin();
	   i != found_photons.end (); ++i)
	{
	  const Vec &dir = (*i)->dir;

#if 0
	  // Incorporate the BSDF response into the photon distribution
	  //
	  Vec bsdf_dir = isec.normal_frame.to (dir);
	  Bsdf::Value bsdf_val
	    = isec.bsdf->eval (bsdf_dir, Bsdf::ALL & ~Bsdf::SPECULAR);

	  Color ph_pow = (*i)->power;
	  Color filt_ph_pow = ph_pow * bsdf_val.val;
	  float filt_ph_intens = filt_ph_pow.intensity();
#else
	  Color ph_pow = (*i)->power;
	  float filt_ph_intens = ph_pow.intensity();
#endif

	  if (filt_ph_intens != 0)
	    {
	      photon_dir_hist.add (dir, filt_ph_intens);
	      all_zero_hist = false;
	    }
	}

      // If we didn't manage to add anything to PHOTON_DIR_HIST,
      // just give up on sampling photons.
      //
      if (all_zero_hist)
	num_photon_samples = 0;
    }

  // Calculate a distribution from PHOTON_DIR_HIST.
  //
  photon_dir_dist.calc (photon_dir_hist);

  // Iterator yielding parameters for photon-direction based sampling.
  //
  std::vector<UV>::const_iterator pi = sample.begin (fgather_photon_chan);

  unsigned bsdf_flags = isec.bsdf->supports ();

  // Shoot NUM_SAMPLES sample rays, using the distribution of nearby photons.
  //
  for (unsigned i = 0; i < num_photon_samples; i++)
    {
      // Sample the photon-direction distribution.
      //
      float samp_pdf;
      Vec samp_dir = photon_dir_dist.sample (*pi++, samp_pdf);

      // Transform SAMP_DIR into ISEC's surface-normal coordinate system.
      //
      Vec bsdf_dir = isec.normal_frame.to (samp_dir);

      // Evaluate the BSDF in the chosen direction.
      //
      Bsdf::Value bsdf_val
	= isec.bsdf->eval (bsdf_dir, Bsdf::ALL & ~Bsdf::SPECULAR);

      // Note that it's extremely rare for SAMP_PDF to be zero, but
      // it can happen, basically when PHOTON_DIR_DIST is all zero.
      //
      if (bsdf_val.pdf != 0 && samp_pdf != 0)
	{
	  // We calculate a separate scale factor for direct surface
	  // emission via specular recursion, which represents the
	  // "caustic" case.
	  //
	  Color indir_emission_scale;
	  if (avoid_caustics_on_diffuse)
	    {
	      // If we're not generating caustics, then for the diffuse
	      // portion of the BSDF, this is zero, because caustics on
	      // diffuse surfaces is handled using the caustics map, so
	      // the scale factor we use is basically everything
	      // _except_ the BSDF's diffuse layer.
	      //
	      // To save a bit of time, though, we only call Bsdf::eval
	      // if the BSDF actually has a non-diffuse layer.
	      
	      if (bsdf_flags
		  & Bsdf::SURFACE_CLASS
		  & ~(Bsdf::SPECULAR|Bsdf::DIFFUSE))
		{
		  // The BSDF has a non-diffuse, non-specular, layer
		  // which might conceivably be a transmission path for
		  // the "caustic case", so set INDIR_EMISSION_SCALE to
		  // filter out the diffuse layer, and keep the rest.

		  Bsdf::Value bsdf_no_diffuse_val
		    = isec.bsdf->eval (
				   bsdf_dir,
				   Bsdf::ALL & ~(Bsdf::SPECULAR|Bsdf::DIFFUSE));

		  // As emission will also be multiplied through the
		  // same BSDF scaling as other light sources, which
		  // includes both diffuse and non-diffuse layers,
		  // INDIR_EMISSION_SCALE is actually a correction
		  // factor which essentially removes the diffuse layer.
		  //
		  // The overall scale factor is (DIFF + NON_DIFF), and
		  // we want just NON_DIFF for emission, so
		  // INDIR_EMISSION_SCALE is NON_DIFF / (DIFF +
		  // NON_DIFF); when multiplied by (DIFF + NON_DIFF),
		  // this will yield just NON_DIFF.
		  //
		  // [Note that Color::operator/ allows zero
		  // denominators, which just yield zero, so we
		  // don't need to guard against divide-by-zero
		  // here.]
		  //
		  indir_emission_scale = bsdf_no_diffuse_val.val / bsdf_val.val;
		}
	      else
		{
		  // The BSDF only has a diffuse layer, so just
		  // ignore any direct emission; it will all be
		  // handled using the caustic map.

		  indir_emission_scale = 0;
		}
	    }
	  else
	    {
	      // We don't have to avoid caustics, so don't treat
	      // direct emission specially.

	      indir_emission_scale = 1;
	    }

	  // Make a fake Bsdf::Sample to describe this sample.
	  //
	  Bsdf::Sample bsdf_samp (bsdf_val.val, samp_pdf, bsdf_dir, 0);

	  // Incorporate incoming radiance from BSDF_SAMP, applying
	  // the power-heuristic to choose the best of the two types
	  // of sampling we're doing.
	  //
	  radiance
	    += (Lo_fgather_samp (isec, media, bsdf_samp, indir_emission_scale)
		* mis_sample_weight (samp_pdf, num_photon_samples,
				     bsdf_val.pdf, num_bsdf_samples));
	}
    }
  

  //
  // (2) Sample based on the BSDF of the surface at ISEC.
  //

  // Iterator yielding parameters for BSDF sampling.
  //
  std::vector<UV>::const_iterator bi = sample.begin (fgather_bsdf_chan);

  // Shoot NUM_BSDF_SAMPLES sample rays, sampling the BSDF for the directions.
  //
  for (unsigned i = 0; i < num_bsdf_samples; i++)
    {
      // Sample the BSDF.
      //
      Bsdf::Sample bsdf_samp
	= isec.bsdf->sample (*bi++, Bsdf::ALL & ~Bsdf::SPECULAR);

      if (bsdf_samp.pdf != 0)
	{
	  // Find the PDF of this sample's direction in the
	  // photon-direction distribution, which we need for
	  // multiple-importance-sampling.
	  //
	  Vec world_dir = isec.normal_frame.from (bsdf_samp.dir);
	  float ph_dir_pdf = photon_dir_dist.pdf (world_dir);

	  // If we're avoiding caustics, we calculate a separate
	  // scale factor for direct surface emission via specular
	  // recursion, which represents the "caustic" case.  This
	  // is 0 for diffuse samples, because caustics on diffuse
	  // surfaces is handled using the caustics map, and 1 for
	  // non-diffuse samples.
	  //
	  Color indir_emission_scale
	    = ((avoid_caustics_on_diffuse
		&& (bsdf_samp.flags & Bsdf::DIFFUSE))
	       ? 0 : 1);

	  // Incorporate incoming radiance from BSDF_SAMP, applying
	  // the power-heuristic to choose the best of the two types
	  // of sampling we're doing.
	  //
	  radiance
	    += (Lo_fgather_samp (isec, media, bsdf_samp, indir_emission_scale)
		* mis_sample_weight (bsdf_samp.pdf, num_bsdf_samples,
				     ph_dir_pdf, num_photon_samples));
	}
    }

  // Note that we don't need to divide by the number of samples, as
  // that factor is included by the weight returned from
  // mis_sample_weight.

  return radiance;
}


// PhotonInteg::Lo

// This method is called by RecursiveInteg to return any radiance
// not due to specular reflection/transmission or direct emission.
//
Color
PhotonInteg::Lo (const Intersect &isec, const Media &media,
		 const SampleSet::Sample &sample)
{
  // True if we're using final-gathering.
  //
  bool use_fgather = (global.num_fgather_samples > 0);

  // True if we're using the caustics-map for caustics on diffuse
  // surfaces.
  //
  bool use_caustics_map = (global.caustic_photon_map.size () != 0);

  Color radiance = 0;

  // Direct-lighting.
  //
  if (global.use_direct_illum)
    radiance += direct_illum.sample_lights (isec, sample);
  else
    radiance += Lo_photon (isec, global.direct_photon_map, global.direct_scale);

  // Caustics.  If final-gathering is enabled, we only evaluate caustics
  // for diffuse reflection, as the non-diffuse case is (better) handled
  // by sampling the BSDF in Lo_fgather.
  //
  if (use_caustics_map)
    radiance
      += Lo_photon (isec, global.caustic_photon_map, global.caustic_scale,
		    use_fgather ? Bsdf::SAMPLE_DIR|Bsdf::DIFFUSE : Bsdf::ALL);

  // Indirect lighting.
  //
  if (use_fgather)
    radiance
      += Lo_fgather (isec, media, sample, use_caustics_map);
  else
    radiance
      += Lo_photon (isec, global.indirect_photon_map, global.indirect_scale);

  return radiance;
}
