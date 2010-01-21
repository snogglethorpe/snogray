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
  : SurfaceInteg::GlobalState (_scene)
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
SurfaceInteg *
OldInteg::GlobalState::make_integrator (RenderContext &context)
{
  return new OldInteg (context, *this);
}
