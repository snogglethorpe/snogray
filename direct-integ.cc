// direct-integ.cc -- Direct-lighting-only surface integrator
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

#include "scene.h"
#include "bsdf.h"

#include "direct-integ.h"


using namespace snogray;



// Constructors etc

DirectInteg::GlobalState::GlobalState (const GlobalRenderState &rstate,
				       const ValTable &params)
  : SurfaceInteg::GlobalState (rstate), direct_illum (params)
{
}

// Integrator state for rendering a group of related samples.
//
DirectInteg::DirectInteg (RenderContext &context, GlobalState &global_state)
  : SurfaceInteg (context), direct_illum (context, global_state.direct_illum)
{
}

// Return a new integrator, allocated in context.
//
SurfaceInteg *
DirectInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new DirectInteg (context, *this);
}


// DirectInteg::Lo

// Return the light emitted from ISEC.
//
Color
DirectInteg::Lo (const Intersect &isec, const Media &media,
		 const SampleSet::Sample &sample, unsigned depth)
  const
{
  // Start out by including any light emitted from the material
  // itself.
  //
  Color radiance = isec.material->Le (isec);

  // Now if there's a BSDF, add contributions from incoming light
  // reflected-from / transmitted-through the surface.  [Only weird
  // materials like light-emitters don't have a BSDF.]
  //
  if (isec.bsdf)
    {
      // Include non-specular direct lighting.
      //
      radiance += direct_illum.sample_lights (isec, sample);

      //
      // If the BSDF includes specular components, recurse to handle those.
      //
      // Note that because there's only one possible specular sample for
      // each direction, we just pass a dummy (0,0) parameter to
      // Bsdf::sample.
      //

      // Try reflection.
      //
      Bsdf::Sample refl_samp
	= isec.bsdf->sample (UV(0,0), Bsdf::SPECULAR|Bsdf::REFLECTIVE);
      if (refl_samp.val > 0)
	{
	  Ray refl_ray (isec.normal_frame.origin,
			isec.normal_frame.from (refl_samp.dir),
			1.f);

	  radiance +=
	    Li (refl_ray, media, sample, depth + 1)
	    * refl_samp.val
	    * abs (isec.cos_n (refl_samp.dir));
	}

      // Try refraction.
      //
      Bsdf::Sample xmit_samp
	= isec.bsdf->sample (UV(0,0), Bsdf::SPECULAR|Bsdf::TRANSMISSIVE);
      if (xmit_samp.val > 0)
	{
	  Media xmit_media (isec, true);

	  Ray ray (isec.normal_frame.origin,
		   isec.normal_frame.from (xmit_samp.dir),
		   1.f);

	  radiance +=
	    Li (ray, xmit_media, sample, depth + 1)
	    * xmit_samp.val
	    * abs (isec.cos_n (xmit_samp.dir));
	}
    }

  return radiance;
}


// DirectInteg::Li

// Return the light arriving at RAY's origin from the direction it
// points in (the length of RAY is ignored).  MEDIA is the media
// environment through which the ray travels.
//
// This method also calls the volume-integrator's Li method, and
// includes any light it returns for RAY as well.
//
// "Li" means "Light incoming".
//
// This an internal variant of Integ::lo which has an additional DEPTH
// argument.  If DEPTH is greater than some limit, recursion will
// stop.  It also returns a Color instead of a Tint, as alpha values
// are only meaningful at the the top-level.
//
Color
DirectInteg::Li (const Ray &ray, const Media &media,
		 const SampleSet::Sample &sample,
		 unsigned depth)
  const
{
  if (depth > 5) return 0; // XXX use russian roulette

  const Scene &scene = context.scene;

  Ray isec_ray (ray, context.params.min_trace, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

  Color radiance;
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (media, context);
      radiance = Lo (isec, media, sample, depth);
    }
  else
    radiance = scene.background (isec_ray);

  // Apply the volume integrator.  It can both filter the radiance from
  // the surface, and add radiance of its own.  Note that in the case
  // where the background was used, the length of ISEC_RAY will be the
  // "scene horizon".
  //
  radiance *= context.volume_integ->transmittance (isec_ray, media.medium);
  radiance += context.volume_integ->Li (isec_ray, media.medium, sample);

  return radiance;
}

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
DirectInteg::Li (const Ray &ray, const Media &media,
		 const SampleSet::Sample &sample)
{
  const Scene &scene = context.scene;

  Ray isec_ray (ray, context.params.min_trace, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

  Color radiance;
  float alpha;
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (media, context);
      radiance = Lo (isec, media, sample, 0);
      alpha = 1;
    }
  else
    {
      radiance = scene.background (isec_ray);
      alpha = scene.bg_alpha;
    }

  // Apply the volume integrator.  It can both filter the radiance from
  // the surface, and add radiance of its own.  Note that in the case
  // where the background was used, the length of ISEC_RAY will be the
  // "scene horizon".
  //
  radiance *= context.volume_integ->transmittance (isec_ray, media.medium);
  radiance += context.volume_integ->Li (isec_ray, media.medium, sample);

  if (radiance != 0)
    alpha = 1;

  return Tint (radiance, alpha);
}
