// global-render-state.h -- global information used during rendering
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

#ifndef __GLOBAL_RENDER_STATE_H__
#define __GLOBAL_RENDER_STATE_H__

#include "space-builder.h"
#include "sample-gen.h"
#include "surface-integ.h"
#include "volume-integ.h"
#include "val-table.h"
#include "unique-ptr.h"


namespace snogray {


// Global state; this contains various read-only global information,
// which will be shared by all rendering threads.
//
class GlobalRenderState
{
public:

  GlobalRenderState (const Scene &_scene, const ValTable &_params);

  // Scene being rendered.  This is also stored in the GLOBAL_STATE object,
  // but we duplicate the info here, as it's so often used.
  //
  const Scene &scene;

  // Number of samples per pixel used for rendering.
  //
  unsigned num_samples;

  // A table of named parameters that can affect rendering.
  //
  const ValTable &params;

  // Sample generator.
  //
  UniquePtr<SampleGen> sample_gen;

  // Factory used to create SpaceBuilder objects when creating a new
  // geometry accelerator.
  //
  UniquePtr<SpaceBuilderFactory> space_builder_factory;

  // Global state for surface integrators.  This should be one of the last
  // fields, so it will be initialized after other fields -- the integrator
  // creation method is passed a reference to the GlobalRenderState object, so
  // we want as much GlobalRenderState state as possible to be valid at that
  // point.
  //
  UniquePtr<SurfaceInteg::GlobalState> surface_integ_global_state;

  // Global state for volume integrators.  This should be one of the last
  // fields, so it will be initialized after other fields -- the integrator
  // creation method is passed a reference to the GlobalRenderState object, so
  // we want as much GlobalRenderState state as possible to be valid at that
  // point.
  //
  UniquePtr<VolumeInteg::GlobalState> volume_integ_global_state;

private:

  //
  // Helper methods, which basically create and return an appropriate
  // object based on what's in PARAMS.
  //
  static SampleGen *make_sample_gen (const ValTable &params);
  static SpaceBuilderFactory *make_space_builder_factory (
				const ValTable &params);
  static SurfaceInteg::GlobalState *make_surface_integ_global_state (
				      const Scene &scene,
				      const ValTable &params);
  static VolumeInteg::GlobalState *make_volume_integ_global_state (
				     const Scene &scene,
				     const ValTable &params);
};


}

#endif // __GLOBAL_RENDER_STATE_H__
