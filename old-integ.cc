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
#include "illum-mgr.h"

#include "old-integ.h"

using namespace snogray;


OldInteg::GlobalState::GlobalState (const Scene &_scene, const ValTable &params)
  : SurfaceInteg::GlobalState (_scene),
    root_cache (_scene.num_lights ())
{
  std::string algo = params.get_string ("algo", "rt");

  if (algo == "ppt" || algo == "pure-path-trace" || algo == "purepathtrace")
    illum_mgr.add_illum (new RecursIllum (scene), 0);
  else
    {
      illum_mgr.add_illum (new MisIllum (scene), IllumSample::DIRECT);

      if (algo == "pt" || algo == "path-trace" || algo == "pathtrace")
	illum_mgr.add_illum (new RecursIllum (scene), 0);
      else if (algo == "rt" || algo == "ray-trace" || algo == "raytrace")
	illum_mgr.add_illum (new RecursIllum (scene), IllumSample::SPECULAR);
      else
	throw std::runtime_error ("Unknown algorithm \"" + algo + "\"");
    }
}


// Return a new integrator, allocated in context.
//
Integ *
OldInteg::GlobalState::make_integrator (SampleSet &samples,
					RenderContext &context)
{
  return new OldInteg (samples, context, *this);
}


// Return light from the scene arriving from the direction of RAY at its
// origin.  SAMPLE_NUM is the sample to use.
//
Tint
OldInteg::li (const Ray &ray, unsigned sample_num)
{
  Ray intersected_ray (ray);
  IsecCtx isec_ctx (context);
  const Surface::IsecInfo *isec_info
    = global.scene.intersect (intersected_ray, isec_ctx);

  Tint tint;
  if (isec_info)
    {
      Trace camera_trace (isec_info->ray, context);
      return global.illum_mgr.li (isec_info, camera_trace);
    }
  else
    return global.scene.background_with_alpha (ray);
}
