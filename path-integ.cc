// path-integ.cc -- Path-tracing surface integrator
//
//  Copyright (C) 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "material/bsdf.h"
#include "scene.h"
#include "material/media.h"
#include "material/material.h"
#include "photon-shooter.h"

#include "path-integ.h"


using namespace snogray;



// PathInteg::PhotonShooter class

class PathInteg::Shooter : public PhotonShooter
{
public:

  Shooter (unsigned num)
    : PhotonShooter ("path-integ"), photon_set (num, "photons", *this)
  {
  }

  // Deposit (or ignore) the photon PHOTON in some photon-set.
  // ISEC is the intersection where the photon is being stored, and
  // BSDF_HISTORY is the bitwise-or of all BSDF past interactions
  // since this photon was emitted by the light (it will be zero for
  // the first intersection).
  //
  virtual void deposit (const Photon &photon, const Intersect &isec,
			unsigned bsdf_history)
  {
    // We only deposit photons on diffuse surfaces, and only for
    // indirect illumination.
    //
    if (!photon_set.complete ()
	&& isec.bsdf->supports (Bsdf::ALL_DIRECTIONS | Bsdf::DIFFUSE))
      photon_set.photons.push_back (photon);
  }

  PhotonSet photon_set;
};


// Constructors etc

PathInteg::GlobalState::GlobalState (const GlobalRenderState &rstate,
				     const ValTable &params)
  : SurfaceInteg::GlobalState (rstate),
    min_path_len (params.get_uint ("min_path_len", 3)),
    max_path_len (params.get_uint ("max_path_len", 25)),
    direct_illum (
      params.get_uint ("direct_samples,dir_samples,dir_samps",
		       rstate.params.get_uint ("direct_samples", 1))),
    photon_eval (
      params.get_uint ("render_photons", 50),
      params.get_float ("photon_radius,radius", 5),
      params.get_float ("marker_radius", 0))
{
  // Shoot photons if the user has enabled "photon-diffuse" mode.
  //
  // [It's disabled by default, because there are some annoying
  // photon artifacts like edge-leakage etc.]
  //
  if (params.get_bool ("photon_diffuse", false))
    {
      // Generate a photon-map to guide rendering.
      //
      Shooter photon_shooter (params.get_uint ("photons", 500000));

      photon_shooter.shoot (rstate);
      photon_map.set_photons (photon_shooter.photon_set.photons);

      if (photon_shooter.photon_set.num_paths > 0)
	photon_scale = 1 / float (photon_shooter.photon_set.num_paths);
    }
}

// Integrator state for rendering a group of related samples.
//
PathInteg::PathInteg (RenderContext &context, GlobalState &global_state)
  : SurfaceInteg (context),
    global (global_state),
    random_sample_set (1, context.samples.gen, context.random),
    random_direct_illum (random_sample_set, context, global.direct_illum),
    photon_eval (context, global_state.photon_eval)
{
  vertex_direct_illums.reserve (global.min_path_len);
  bsdf_sample_channels.reserve (global.min_path_len);

  for (unsigned i = 0; i < global.min_path_len; i++)
    {
      vertex_direct_illums.push_back (
			     DirectIllum (context, global_state.direct_illum));
      bsdf_sample_channels.push_back (
			     context.samples.add_channel<UV> ());
    }
}

// Return a new integrator, allocated in context.
//
SurfaceInteg *
PathInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new PathInteg (context, *this);
}


// PathInteg::Li

// Return the light arriving at RAY's origin from the direction it
// points in (the length of RAY is ignored).  MEDIA is the media
// environment through which the ray travels.
//
// This method also calls the volume-integrator's Li method, and
// includes any light it returns for RAY as well.
//
// "Li" means "Light incoming".
//
Tint
PathInteg::Li (const Ray &ray, const Media &orig_media,
	       const SampleSet::Sample &sample)
{
  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  // The innermost media layer in a stack of media layers active at the
  // current vertex.  A new layer is pushed when entering a refractive
  // object, and the top layer is popped when exiting a refractive object.
  //
  const Media *innermost_media = &orig_media;

  Ray isec_ray (ray, scene.horizon);

  // Length of the current path.
  //
  unsigned path_len = 0;

  // The transmittance of the entire current path from the beginning to the
  // current vertex.  Each new vertex will make this smaller because of the
  // filtering effect of the BSDF at that location.
  //
  Color path_transmittance = 1;

  // True if we followed a specular sample from the previous path
  // vertex.
  //
  bool after_specular_sample = false;

  // We acculate the outgoing illumination in RADIANCE.
  //
  Color radiance = 0;

  // The alpha value; this is always 1 except in the case where a camera
  // ray directly hits the scene background.
  //
  float alpha = 1;

  // Grow the path, one vertex at a time.  At each vertex, the lighting
  // contribution will be added for that vertex, and then a new sample
  // direction is chosen to use for the path's next vertex.  This will
  // terminate only when the path fails to hit anything, it hits a
  // completely non-reflecting, non-transmitting surface, or is
  // terminated prematurely by russian-roulette.
  //
  for (;;)
    {
      const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

      // Top of current media stack.
      //
      const Media &media = *innermost_media;

      // Include lighting from the volume integrator.  Note that we do
      // this before updating PATH_TRANSMITTANCE, because
      // VolumeInteg::Li should handle attentuation.
      //
      radiance
	+= (context.volume_integ->Li (isec_ray, media.medium, sample)
	    * path_transmittance);

      // Update PATH_TRANSMITTANCE to reflect any attenuation over ISEC_RAY.
      //
      path_transmittance
	*= context.volume_integ->transmittance (isec_ray, media.medium);

      // Normally, we don't add light emitted by a surface we hit, or
      // background light in the case we don't hit a surface, because
      // that should have been accounted for by the direct-lighting term
      // in the _previous_ path-vertex.
      //
      // However in the special cases of (1) the first path-vertex
      // (representing the first intersection after a camera ray),
      // (2) a vertex following a specular reflection/refraction, or
      // (3) the direct-lighting optimization is disabled, we _do_
      // add light emitted, because in these cases there is no
      // previous-vertex direct-lighting term.
      //
      bool include_emitters
	= (path_len == 0
	   || after_specular_sample
	   || global.direct_illum.num_samples == 0);

      // If we didn't hit anything, terminate the path.
      //
      if (! isec_info)
	{
	  // If we're including emitters, then add scene background
	  // light.
	  //
	  if (include_emitters)
	    radiance += scene.background (isec_ray) * path_transmittance;

	  if (path_len == 0 && radiance == 0)
	    alpha = context.global_state.bg_alpha;

	  // Terminate the path.
	  //
	  break;
	}

      // Generate a new Intersect object for the intersection at this
      // path-vertex.
      //
      Intersect isec = isec_info->make_intersect (media, context);

      // If we're including emitters, then add light emitted by this
      // surface.
      //
      if (include_emitters)
	radiance += isec.material.Le (isec) * path_transmittance;

      // If there's no BSDF at all, this path is done.
      //
      if (! isec.bsdf)
	break;

      // If we have a non-empty photon-map, use it for diffuse indirect
      // lighting.
      //
      unsigned non_photon_flags = Bsdf::ALL;
      if (path_len != 0
	  && !after_specular_sample
	  && global.photon_map.size () != 0)
	{
	  radiance
	    += (photon_eval.Lo (isec, global.photon_map, global.photon_scale,
				Bsdf::ALL_DIRECTIONS | Bsdf::DIFFUSE)
		* path_transmittance);

	  // Omit layers we evaluated using the photon-map below.
	  //
	  non_photon_flags &= ~Bsdf::DIFFUSE;
	}

      // Include direct lighting (if enabled).  Note that this
      // explicitly omits specular samples.
      //
      if (global.direct_illum.num_samples != 0)
	{
	  unsigned dir_flags = non_photon_flags & ~Bsdf::SPECULAR;

	  if (path_len < global.min_path_len)
	    //
	    // For path-vertices near the beginning, use pre-generated
	    // (and well-distributed) samples from SAMPLE.
	    //
	    radiance
	      += (vertex_direct_illums[path_len].sample_lights (isec, sample,
								dir_flags)
		  * path_transmittance);
	  else
	    //
	    // For path-vertices not near the beginning, generate new
	    // random samples every time.
	    {
	      // Make more samples for RANDOM_DIRECT_ILLUM.
	      //
	      random_sample_set.generate ();

	      SampleSet::Sample random_sample (random_sample_set, 0);

	      radiance
		+= (random_direct_illum.sample_lights (isec, random_sample,
						       dir_flags)
		    * path_transmittance);
	    }
	}

      // Choose a parameter for sampling the BSDF.  For path vertices
      // near the beginning (PATH_LEN < MIN_PATH_LEN), we use
      // SampleSet::Sample::get to get a sample from SAMPLE; if we've
      // MIN_PATH_LEN, then just generate a completely random sample
      // instead.
      //
      UV bsdf_samp_param =
	((path_len < global.min_path_len)
	 ? sample.get (bsdf_sample_channels[path_len])
	 : UV (context.random (), context.random ()));

      // Now sample the BSDF to get a new ray for the next path vertex.
      //
      Bsdf::Sample bsdf_samp
	= isec.bsdf->sample (bsdf_samp_param, non_photon_flags);

      // If the BSDF couldn't give us a sample, this path is done.
      // It's essentially perfect  black.
      //
      if (bsdf_samp.pdf == 0 || bsdf_samp.val == 0)
	break;

      // Add this BSDF sample to PATH_TRANSMITTANCE.
      //
      path_transmittance
	*= bsdf_samp.val * abs (isec.cos_n (bsdf_samp.dir)) / bsdf_samp.pdf;

      // If this path is getting long, use russian roulette to randomly
      // terminate it.
      //
      if (path_len > global.min_path_len)
	{
	  // RR_CONTINUE_PROB is the probability that we'll continue
	  // this path.
	  //
	  // We make it proportional to the current path transmittance
	  // so that paths with high-transmittance, which have a
	  // bigger effect on the final result, will be explored
	  // farther.
	  //
	  float rr_continue_prob = min (1.f, path_transmittance.intensity ());
	  float russian_roulette = context.random ();
	  
	  if (russian_roulette > rr_continue_prob)
	    //
	    // Terminated!
	    break;
	  else
	    // Don't terminate.  Adjust PATH_TRANSMITTANCE to reflect
	    // the fact that we tried.
	    //
	    // By dividing by the probability of continuation, which is
	    // less than 1, we boost the intensity of paths that survive
	    // russian-roulette, which will exactly compensate for the
	    // zero value of paths that are terminated by it.
	    //
	    path_transmittance /= rr_continue_prob;
	}
      if (path_len == global.max_path_len)
	break;

      // Update ISEC_RAY to point from ISEC's position in the direction
      // of the BSDF sample.  
      //
      isec_ray = Ray (isec.normal_frame.origin,
		      isec.normal_frame.from (bsdf_samp.dir),
		      min_dist, scene.horizon);

      // Remember whether we followed a specular sample, because such
      // samples are normally not accounted for in the direct-lighting
      // term, and so if the sample hits an emitter, the emitter
      // should be included (normally emission terms are ignored
      // because their contribution is accounted for by the preceding
      // direct-lighting term).
      //
      after_specular_sample = (bsdf_samp.flags & Bsdf::SPECULAR);

      // If we just followed a refractive (transmissive) sample, we need
      // to update our stack of Media entries:  entering a refractive
      // object pushes a new Media, existing one pops the top one.
      //
      if (bsdf_samp.flags & Bsdf::TRANSMISSIVE)
	Media::update_stack_for_transmission (innermost_media, isec);

      path_len++;
    }

  return Tint (radiance, alpha);
}
