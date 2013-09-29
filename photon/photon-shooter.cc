// photon-shooter.cc -- Photon-shooting infrastructure
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "light/light.h"
#include "material/media.h"
#include "scene/scene.h"
#include "material/bsdf.h"
#include "util/radical-inverse.h"
#include "render/render-context.h"
#include "render/global-render-state.h"

#include "cli/tty-progress.h"
#include "util/string-funs.h"

#include "photon-shooter.h"

using namespace snogray;


// Shoot photons from the lights, depositing them in photon-sets at
// appropriate points.
//
void
PhotonShooter::shoot (const GlobalRenderState &global_render_state)
{
  RenderContext context (global_render_state);
  Media surrounding_media (context.default_medium);

  const std::vector<const Light::Sampler *> &light_samplers
    = context.scene.light_samplers;

  if (light_samplers.size () == 0)
    return;			// no lights, so no point

  TtyProgress prog (std::cout, "* " + name + ": shooting photons...");

  prog.set_size (target_count ());
  prog.start ();

  for (unsigned path_num = 0; ! complete(); path_num++) 
    {
      prog.update (cur_count ());

      // Randomly choose a light-sampler.
      //
      unsigned sampler_num
	= radical_inverse (path_num, 11) * light_samplers.size ();
      const Light::Sampler *light_sampler = light_samplers[sampler_num];

      // Sample the light.
      //
      UV pos_param (radical_inverse (path_num, 2),
		    radical_inverse (path_num, 3));
      UV dir_param (radical_inverse (path_num, 5),
		    radical_inverse (path_num, 7));
      Light::Sampler::FreeSample samp
	= light_sampler->sample (pos_param, dir_param);
      
      if (samp.val == 0 || samp.pdf == 0)
	continue;

      // Update the number of paths generated.  Every light sample is a
      // potential photon path for all photon types that haven't
      // finished yet (we do all types in parallel).
      //
      for (std::vector<PhotonSet *>::iterator psi = photon_sets.begin();
	   psi != photon_sets.end(); ++psi)
	if (! (*psi)->complete ())
	  (*psi)->num_paths++;

      // The logical-or of all the Bsdf::ALL_LAYERS flags we
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
      Color power = samp.val * float (light_samplers.size ()) / samp.pdf;

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
	  const Surface::Renderable::IsecInfo *isec_info
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

	  // The photon we're going to store.  Note that the
	  // direction is reversed, as the photon's direction points
	  // to where it _came_ from.
	  //
	  Photon photon (isec.normal_frame.origin, -dir, power);

	  // Now maybe deposit a photon at this location.  This is done by
	  // calling a subclass-specific method, which may want to 
	  //
	  deposit (photon, isec, bsdf_history);

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
	  power
	    *= bsdf_samp.val * abs (isec.cos_n (bsdf_samp.dir)) / bsdf_samp.pdf;

	  // Remember the type of reflection/refraction in our history.
	  //
	  // We don't record any history for "translucent" samples, as
	  // they are generally treated as if they come directly from
	  // the light.
	  //
	  if (! (bsdf_samp.flags & Bsdf::TRANSLUCENT))
	    bsdf_history |= bsdf_samp.flags;

	  // If we just followed a refractive (transmissive) sample, we need
	  // to update our stack of Media entries:  entering a refractive
	  // object pushes a new Media, existing one pops the top one.
	  //
	  if (bsdf_samp.flags & Bsdf::TRANSMISSIVE)
	    Media::update_stack_for_transmission (innermost_media, isec);
	}

      context.mempool.reset ();

      if (path_num > 1e8)
	break;
    }

  prog.end ();

  // Output information message about results.
  //
  bool some = false;
  std::cout << "* " << name << ": ";
  for (std::vector<PhotonSet *>::iterator psi = photon_sets.begin();
       psi != photon_sets.end(); ++ psi)
    {
      PhotonSet &ps = **psi;
      if (ps.photons.size () != 0)
	{
	  if (some)
	    std::cout << ", ";  
	  std::cout << commify (ps.photons.size ()) << " " << ps.name;
	  std::cout << " (" << commify (ps.num_paths) << " paths)";
	  some = true;
	}
    }
  if (! some)
    std::cout << "no photons generated!";
  std::cout << std::endl;
}
