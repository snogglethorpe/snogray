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
#include "brdf.h"

#include "direct-integ.h"


using namespace snogray;



// Constructors etc

DirectInteg::GlobalState::GlobalState (const Scene &scene,
				       const ValTable &params)
  : SurfaceInteg::GlobalState (scene), direct_illum (params)
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

// Return the color emitted from the ray-surface intersection ISEC.
// "Lo" means "Light outgoing".
//
// This an internal variant of Integ::lo which has an additional DEPTH
// argument.  If DEPTH is greater than some limit, recursion will stop.
//
Color
DirectInteg::Lo (const Intersect &isec, const SampleSet::Sample &sample,
		 unsigned depth)
  const
{
  // Start out by including any light emitted from the material itself.
  //
  Color radiance = isec.material->le (isec);

  // Now if there's a BRDF, add contributions from incoming light
  // reflected-from / transmitted-through the surface.  [Only weird
  // materials like light-emitters don't have a BRDF.]
  //
  if (isec.brdf)
    {
      // Include non-specular direct lighting.
      //
      radiance += direct_illum.sample_lights (isec, sample);

      //
      // If the BRDF includes specular components, recurse to handle those.
      //
      // Note that because there's only one possible specular sample for
      // each direction, we just pass a dummy (0,0) parameter to
      // Brdf::sample.
      //

      // Try reflection.
      //
      Brdf::Sample refl_samp
	= isec.brdf->sample (UV(0,0), Brdf::SPECULAR|Brdf::REFLECTIVE);
      if (refl_samp.val > 0)
	radiance
	  += (Li (isec, refl_samp.dir, sample,
		  Trace::REFLECTION, 0, depth + 1)
	      * refl_samp.val
	      * abs (isec.cos_n (refl_samp.dir)));

      // Try refraction.
      //
      Brdf::Sample xmit_samp
	= isec.brdf->sample (UV(0,0), Brdf::SPECULAR|Brdf::TRANSMISSIVE);
      if (xmit_samp.val > 0)
	{
	  Trace::Type subtrace_type;
	  const Medium *subtrace_medium;

	  if (isec.back)
	    {
	      subtrace_type = Trace::REFRACTION_OUT;
	      subtrace_medium
		= &isec.trace.enclosing_medium (context.default_medium);
	    }
	  else
	    {
	      subtrace_type = Trace::REFRACTION_IN;
	      subtrace_medium = isec.material->medium ();
	    }

	  radiance
	    += (Li (isec, xmit_samp.dir, sample,
		    subtrace_type, subtrace_medium,
		    depth + 1)
		* xmit_samp.val
		* abs (isec.cos_n (xmit_samp.dir)));
	}
    }

  return radiance;
}


// DirectInteg::Li

// Return the light hitting TARGET_ISEC from direction DIR; DIR is in
// TARGET_ISEC's surface-normal coordinate-system.  SUBTRACE_TYPE and
// SUBTRACE_MEDIUM describe the type of transition represented by the new
// ray, and the medium it has entered.  If SUBTRACE_MEDIUM is 0, then
// TARGET_ISEC's medium is used instead.
//
Color
DirectInteg::Li (const Intersect &target_isec, const Vec &dir,
		 const SampleSet::Sample &sample,
		 Trace::Type subtrace_type,
		 const Medium *subtrace_medium,
		 unsigned depth)
  const
{
  if (depth > 5) return 0; // XXX use russian roulette

  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  Ray isec_ray (target_isec.normal_frame.origin,
		target_isec.normal_frame.from (dir),
		min_dist, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

  Color radiance;
  if (isec_info)
    {
      if (! subtrace_medium)
	subtrace_medium = &target_isec.trace.medium;

      Trace trace (subtrace_type, isec_info->ray, *subtrace_medium, 1.f,
		   target_isec.trace);

      Intersect isec = isec_info->make_intersect (trace, context);

      radiance = Lo (isec, sample, depth);
    }
  else
    radiance = scene.background_with_alpha (isec_ray).alpha_scaled_color();

  radiance *= context.volume_integ->transmittance (isec_ray,
						   context.default_medium);

  radiance += context.volume_integ->Li (isec_ray,
					context.default_medium,
					sample);

  return radiance;
}
