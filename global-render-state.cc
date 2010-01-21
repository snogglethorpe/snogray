// global-render-state.cc -- global information used during rendering
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

#include "mis-illum.h"
#include "octree.h"
#include "grid.h"
#include "old-integ.h"

#include "global-render-state.h"


using namespace snogray;


GlobalRenderState::GlobalRenderState (const Scene &_scene,
				      const ValTable &_params)
  : scene (_scene),
    num_samples (_params.get_uint ("oversample", 1)),
    params (_params),
    sample_gen (make_sample_gen (_params)),
    space_builder_factory (make_space_builder_factory (_params)),
    surface_integ_global_state (
      make_surface_integ_global_state (_scene, _params))
{ }


//
// Helper methods, which basically create and return an appropriate
// object based on what's in PARAMS.
//

SampleGen *
GlobalRenderState::make_sample_gen (const ValTable &)
{
  return new Grid;
}

SpaceBuilderFactory *
GlobalRenderState::make_space_builder_factory (const ValTable &)
{
  return new Octree::BuilderFactory;
}

SurfaceInteg::GlobalState *
GlobalRenderState::make_surface_integ_global_state (const Scene &scene, const ValTable &params)
{
  return new OldInteg::GlobalState (scene, params);
}
