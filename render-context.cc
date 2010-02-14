// render-context.cc -- "semi-global" information used during rendering
//
//  Copyright (C) 2006, 2007, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "octree.h"

#include "render-context.h"


using namespace snogray;

RenderContext::RenderContext (const GlobalRenderState &_global_state)
  : scene (_global_state.scene),
    samples (_global_state.num_samples, *_global_state.sample_gen, random),
    global_state (_global_state),
    params (_global_state.params),
    surface_integ (
      _global_state.surface_integ_global_state
      ? _global_state.surface_integ_global_state->make_integrator (*this)
      : 0),
    volume_integ (
      _global_state.volume_integ_global_state
      ? _global_state.volume_integ_global_state->make_integrator (*this)
      : 0)
{ }

RenderContext::~RenderContext ()
{
}


// arch-tag: bacb1ae7-4350-408b-8746-cbcf77debc30
