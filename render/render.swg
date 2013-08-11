# render.swg -- SWIG interfaces for snogray rendering types
#
#  Copyright (C) 2011-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "render/global-render-state.h"
#include "render/render-context.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class GlobalRenderState
  {
  public:

    GlobalRenderState (const snogray::Scene &_scene,
		      const snogray::ValTable &_params);
  };


  // A wrapper for RenderStats::IsecStats (SWIG can't handle nested
  // classes).
  //
  struct IsecStats
  {
    unsigned long long surface_intersects_tests;
    unsigned long long surface_intersects_hits;
    unsigned long long neg_cache_hits;
    unsigned long long neg_cache_collisions;
    unsigned long long space_node_intersect_calls;
  };
  %{
  namespace snogray {
    struct IsecStats : public RenderStats::IsecStats { };
  }
  %}

  struct RenderStats
  {
    RenderStats ();

    unsigned long long scene_intersect_calls;
    unsigned long long scene_shadow_tests;
    unsigned long long illum_calls;

    IsecStats intersect, shadow;
  };


  class RenderContext
  {
  public:

    RenderContext (const snogray::GlobalRenderState &global_state);
    ~RenderContext ();

    const Scene &scene;

    // Medium default_medium;

    RenderStats stats;

    // Random random;

    const snogray::GlobalRenderState &global_state;

    // const RenderParams params;
    // UniquePtr<SurfaceInteg> surface_integ;
    // UniquePtr<VolumeInteg> volume_integ;
  };


} // namespace snogray
