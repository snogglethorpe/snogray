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

#include "photon-integ.h"


using namespace snogray;



// Constructors etc

PhotonInteg::GlobalState::GlobalState (const GlobalRenderState &rstate,
				       const ValTable &params)
  : RecursiveInteg::GlobalState (rstate),
    num_radiance_photons (
      params.get_uint ("surface-integ.photon.num", 50)),
    photon_search_radius (
      params.get_float ("surface-integ.photon.radius", 0.1)),
    caustic_scale (0), direct_scale (0), indirect_scale (0),
    marker_radius_sq (
      params.get_float ("surface-integ.photon.marker-radius", 0)),
    direct_illum (
      params.get_uint ("light-samples,surface-integ.photon.direct-samples",
		       16)),
    use_direct_illum (
      params.get_bool (
	       "surface-integ.photon.direct-illum,surface-integ.photon.dir-illum",
	       true))
{
  unsigned num_caustic
    = params.get_uint ("surface-integ.photon.caustic", 20000);
  unsigned num_direct
    = params.get_uint (
	       "surface-integ.photon.direct,surface-integ.photon.dir",
	       100000);
  unsigned num_indirect
    = params.get_uint (
	       "surface-integ.photon.indirect,surface-integ.photon.indir",
	       100000);

  marker_radius_sq *= marker_radius_sq;

  generate_photons (num_caustic, num_direct, num_indirect);
}

// Integrator state for rendering a group of related samples.
//
PhotonInteg::PhotonInteg (RenderContext &context, GlobalState &global_state)
  : RecursiveInteg (context), global (global_state),
    num_photons (global_state.num_radiance_photons),
    search_radius (global_state.photon_search_radius),
    direct_illum (context, global_state.direct_illum)
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

  Progress prog (std::cout, "* photon-integ: shooting photons...",
		 0, num_caustic + num_direct + num_indirect);

  prog.start ();

  const std::vector<Light *> &lights = context.scene.lights;

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
	      else if (! (bsdf_history & Bsdf::SURFACE_CLASS & ~Bsdf::SPECULAR))
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

// Return the light emitted from ISEC by photons found nearby in PHOTON_MAP.
// SCALE is the amount by which to scale each photon's radiance.
//
Color
PhotonInteg::Lo_photon (const Intersect &isec, const PhotonMap &photon_map,
			float scale)
{
  if (scale == 0)
    return 0;

  // Give up if this is a purely specular surface.
  //
  if (! isec.bsdf->supports (Bsdf::ALL & ~Bsdf::SPECULAR))
    return 0;

  const Pos &pos = isec.normal_frame.origin;

  found_photons.clear ();
  dist_t max_dist_sq
    = photon_map.find_photons (pos, num_photons, global.photon_search_radius,
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

      // XXXXXXXXXX Ergh, here we need a flags parameter to Bsdf::eval,
      // but there isn't one... for now, just ignore photons that didn't
      // come from above the surface (in other words, only handle
      // reflective BSDFs here).  XXXXXXXXXXXXX
      //
      Vec dir = isec.normal_frame.to (ph.dir);

      if (isec.cos_n (dir) > 0) // XXX ignore photons coming from below surface
	{
	  Bsdf::Value bsdf_val = isec.bsdf->eval (dir);

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

      // XXXX PBRT book avoids calling Bsdf::eval more than once for
      // diffuse surrfaces (since they have a constant
      // value).... worthwhile?  XXXX
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


// PhotonInteg::Lo

// This method is called by RecursiveInteg to return any radiance
// not due to specular reflection/transmission or direct emission.
//
Color
PhotonInteg::Lo (const Intersect &isec, const Media &media,
		 const SampleSet::Sample &sample)
{
  Color radiance = 0;

  // Direct-lighting.
  //
  if (global.use_direct_illum)
    radiance += direct_illum.sample_lights (isec, sample);
  else
    radiance += Lo_photon (isec, global.direct_photon_map, global.direct_scale);

  // Caustics.
  //
  radiance += Lo_photon (isec, global.caustic_photon_map, global.caustic_scale);

  // Indirect lighting.
  //
  // if (do_final_gather)
  // 	...
  // else
  radiance
    += Lo_photon (isec, global.indirect_photon_map, global.indirect_scale);

  return radiance;
}
