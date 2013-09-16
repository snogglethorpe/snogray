// global-render-state.cc -- global information used during rendering
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

#include "util/excepts.h"
#include "space/octree.h"
#include "space/triv-space.h"
#include "grid.h"
#include "direct-integ.h"
#include "path-integ.h"
#include "photon-integ.h"
#include "filter-volume-integ.h"

#include "global-render-state.h"


using namespace snogray;


GlobalRenderState::GlobalRenderState (const Surface &scene_contents,
				      const ValTable &_params)
  : scene (scene_contents,
	   // note: Using UniquePtr ensures the SpaceBuilderFactory
	   // will be deleted after the expression is complete (like
	   // all C++ temporaries, it will be destroyed at the end of
	   // the outermost expression).
	   *UniquePtr<const SpaceBuilderFactory> (
		       make_space_builder_factory (_params))),
    bg_alpha (_params.get_float ("background_alpha", 1)),
    num_samples (_params.get_uint ("samples", 1)),
    params (_params),
    sample_gen (make_sample_gen (_params))
{
  // Set up these separately, as they receive, and may use, our state.
  //
  // We first let them be default-initialized (to null pointers) in the
  // initialization section above, and then set up them one-by-one.
  //
  volume_integ_global_state.reset (make_volume_integ_global_state (_params));
  surface_integ_global_state.reset (make_surface_integ_global_state (_params));
}


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
GlobalRenderState::make_space_builder_factory (const ValTable &params)
{
  std::string accel = params.get_string ("accel", "octree");

  if (accel == "octree")
    return new Octree::BuilderFactory ();
  else if (accel == "triv" || accel == "trivial")
    return new TrivSpace::BuilderFactory ();
  else
    throw std::runtime_error ("Unknown search-accelerator \"" + accel + "\"");
}


SurfaceInteg::GlobalState *
GlobalRenderState::make_surface_integ_global_state (const ValTable &params)
{
  std::string sint = params.get_string ("surface_integ.type", "direct");
  ValTable sint_params = params.readonly_subtable ("surface_integ");

  if (sint == "direct")
    return new DirectInteg::GlobalState (*this, sint_params);
  else if (sint == "path")
    return new PathInteg::GlobalState (*this, sint_params);
  else if (sint == "photon")
    return new PhotonInteg::GlobalState (*this, sint_params);
  else
    throw std::runtime_error ("Unknown surface-integrator \"" + sint + "\"");
}

VolumeInteg::GlobalState *
GlobalRenderState::make_volume_integ_global_state (const ValTable &)
{
  return new FilterVolumeInteg::GlobalState (*this);
}
