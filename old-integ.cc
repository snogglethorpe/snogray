// old-integ.cc -- Surface integrator interface for old rendering system
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
#include "excepts.h"
#include "mis-illum.h"
#include "recurs-illum.h"

#include "old-integ.h"


using namespace snogray;


OldInteg::GlobalState::GlobalState (const Scene &_scene, const ValTable &params)
  : SurfaceInteg::GlobalState (_scene)
{
  std::string algo = params.get_string ("algo", "rt");

  if (algo == "ppt" || algo == "pure-path-media" || algo == "purepathmedia")
    illum_mgr.add_illum (new RecursIllum (scene), 0);
  else
    {
      illum_mgr.add_illum (new MisIllum (scene), IllumSample::DIRECT);

      if (algo == "pt" || algo == "path-media" || algo == "pathtrace")
	illum_mgr.add_illum (new RecursIllum (scene), 0);
      else if (algo == "rt" || algo == "ray-trace" || algo == "raytrace")
	illum_mgr.add_illum (new RecursIllum (scene), IllumSample::SPECULAR);
      else if (algo == "direct")
	;
      else
	throw std::runtime_error ("Unknown algorithm \"" + algo + "\"");
    }
}


// Return a new integrator, allocated in context.
//
SurfaceInteg *
OldInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new OldInteg (context, *this);
}


// OldInteg::Li

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
OldInteg::Li (const Ray &ray, const Media &media,
	      const SampleSet::Sample &sample)
  const
{
  const Scene &scene = context.scene;
  dist_t min_dist = context.params.min_trace;

  Ray isec_ray (ray, min_dist, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (isec_ray, context);

  Tint radiance;		// light from the recursion
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (media, context);
      radiance = global.illum_mgr.Lo (isec);
    }
  else
    radiance = scene.background_with_alpha (isec_ray).alpha_scaled_color();

  radiance *= context.volume_integ->transmittance (isec_ray, media.medium);

  radiance += context.volume_integ->Li (isec_ray, media.medium, sample);

  return radiance;
}
