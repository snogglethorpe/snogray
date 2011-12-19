# rdrive.swg -- SWIG interfaces for "rdrive", the "rendering driver" library
#
#  Copyright (C) 2011  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include <iosfwd>

#include "render-mgr.h"
#include "render-pattern.h"
#include "render-stats.h"
#include "global-render-state.h"
#include "tty-progress.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class ImageOutput;


  class GlobalRenderState
  {
  public:

    GlobalRenderState (const snogray::Scene &_scene,
		      const snogray::ValTable &_params);
  };


  class RenderMgr
  {
  public:

    RenderMgr (const GlobalRenderState &global_state,
	       const snogray::Camera &_camera,
	       unsigned _width, unsigned _height);

    void render (unsigned num_threads,
		 snogray::RenderPattern &pattern, snogray::ImageOutput &output,
		 snogray::Progress &prog, snogray::RenderStats &stats);
  };


  class RenderPattern
  {
  public:

    RenderPattern (int _left_x, int _top_y, int _width, int _height);
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


  class Progress { };
  class TtyProgress : public Progress
  {
  public:

    enum Verbosity { QUIET, MINIMAL, CHATTY };

    static float default_update_interval ();

    TtyProgress (std::iostream &stream, const char *_prefix,
		 Verbosity _verbosity = CHATTY,
		 float _update_interval = default_update_interval());
  };


}
