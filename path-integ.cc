// path-integ.cc -- Path-tracing surface integrator
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

#include "bsdf.h"
#include "scene.h"

#include "path-integ.h"


using namespace snogray;



// Constructors etc

PathInteg::GlobalState::GlobalState (const GlobalRenderState &rstate,
				     const ValTable &params)
  : SurfaceInteg::GlobalState (rstate),
    min_path_len (
      params.get_uint ("surface-integ.path.min-len", 5)),
    russian_roulette_terminate_probability (
      params.get_float ("surface-integ.path.russian-roulette-terminate-probability,surface-integ.path.rr-term-prob,surface-integ.path.rr-term", 0.5f)),
    direct_illum (
      params.get_uint ("light-samples,surface-integ.path.direct-samples", 1))
{
}

// Integrator state for rendering a group of related samples.
//
PathInteg::PathInteg (RenderContext &context, GlobalState &global_state)
  : SurfaceInteg (context),
    global (global_state),
    random_sample_set (1, context.samples.gen, context.random),
    random_direct_illum (random_sample_set, context, global.direct_illum)
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

      // If we didn't hit anything, terminate the path.
      //
      if (! isec_info)
	{
	  // If this is the camera ray, or directly follows a specular
	  // sample, we add the scene background (otherwise the scene
	  // background will have been picked up by the direct-lighting
	  // calculation at the previous path vertext).
	  //
	  if (path_len == 0 || after_specular_sample)
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

      // Normally, we don't add light emitted by the material at a path
      // vertex because that should have been accounted for by the
      // direct-lighting term in the _previous_ vertex.
      //
      // However In the special cases of (1) the first vertex
      // (representing the first intersection after a camera ray), or
      // (2) a vertex following a specular reflection/refraction, we
      // _do_ add light emitted, because in these cases there is no
      // previous-vertex direct-lighting term.
      //
      if (path_len == 0 || after_specular_sample)
	radiance += isec.material->Le (isec) * path_transmittance;

      // If there's no BSDF at all, this path is done.
      //
      if (! isec.bsdf)
	break;

      // Include direct lighting.  Note that this explicitly omits
      // specular samples.
      //
      if (path_len < global.min_path_len)
	//
	// For path-vertices near the beginning, use pre-generated (and
	// well-distributed) samples from SAMPLE.
	//
	radiance
	  += (vertex_direct_illums[path_len].sample_lights (isec, sample)
	      * path_transmittance);
      else
	//
	// For path-vertices not near the beginning, generate new random
	// samples every time.
	{
	  // Make more samples for RANDOM_DIRECT_ILLUM.
	  //
	  random_sample_set.generate ();

	  SampleSet::Sample random_sample (random_sample_set, 0);

	  radiance
	    += (random_direct_illum.sample_lights (isec, random_sample)
		* path_transmittance);
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
      Bsdf::Sample bsdf_samp = isec.bsdf->sample (bsdf_samp_param);

      // If the BSDF couldn't give us a sample, this path is done.
      // It's essentially perfect  black.
      //
      if (bsdf_samp.pdf == 0 || bsdf_samp.val == 0)
	break;

      // If this path is getting long, use russian roulette to randomly
      // terminate it.
      //
      if (path_len > global.min_path_len)
	{
	  float russian_roulette = context.random ();

	  if (russian_roulette < global.russian_roulette_terminate_probability)
	    //
	    // Terminate!
	    //
	    break;
	  else
	    //
	    // Don't terminate.  Adjust PATH_TRANSMITTANCE to reflect
	    // the fact that we tried.
	    //
	    // By dividing by the probability of termination, which is
	    // less than 1, we boost the intensity of paths that survive
	    // russian-roulette, which will exactly compensate for the
	    // zero value of paths that are terminated by it.
	    //
	    path_transmittance
	      /= 1 - global.russian_roulette_terminate_probability;
	}

      // Add this BSDF sample to PATH_TRANSMITTANCE.
      //
      path_transmittance
	*= bsdf_samp.val * abs (isec.cos_n (bsdf_samp.dir)) / bsdf_samp.pdf;

      // Update ISEC_RAY to point from ISEC's position in the direction
      // of the BSDF sample.  
      //
      isec_ray = Ray (isec.normal_frame.origin,
		      isec.normal_frame.from (bsdf_samp.dir),
		      min_dist, scene.horizon);

      // Remember whether we followed a specular sample.
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
