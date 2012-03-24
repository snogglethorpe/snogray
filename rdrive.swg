# rdrive.swg -- SWIG interfaces for "rdrive", the "rendering driver" library
#
#  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
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
#include "tty-progress.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class ImageSampledOutput;
  class GlobalRenderState;


  class RenderMgr
  {
  public:

    RenderMgr (const GlobalRenderState &global_state,
	       const snogray::Camera &_camera,
	       unsigned _width, unsigned _height);

    void render (unsigned num_threads,
		 snogray::RenderPattern &pattern,
		 snogray::ImageSampledOutput &output,
		 snogray::Progress &prog, snogray::RenderStats &stats);
  };


  class RenderPattern
  {
  public:

    RenderPattern (int _left_x, int _top_y, int _width, int _height);
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
