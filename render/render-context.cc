// render-context.cc -- "semi-global" information used during rendering
//
//  Copyright (C) 2006, 2007, 2009, 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/mutex.h"
#include "global-render-state.h"

#include "render-context.h"


using namespace snogray;


// Return an integer which can be used to seed a new random-number
// generator in the current thread.
//
static unsigned
make_rng_seed ()
{
  // No attempt is made to generate a great seed; the main intent is to
  // avoid every thread using the _same_ seed.

  static unsigned global_seed_counter = 0;
  static Mutex global_seed_counter_lock;

  global_seed_counter_lock.lock ();
  unsigned global_count = global_seed_counter++;
  global_seed_counter_lock.unlock ();

  return 578987 + global_count * 1023717;
}


RenderContext::RenderContext (const GlobalRenderState &_global_state)
  : scene (_global_state.scene),
    samples (_global_state.num_samples, *_global_state.sample_gen, random),
    random (make_rng_seed ()),
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
