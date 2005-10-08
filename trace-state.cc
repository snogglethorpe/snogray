// trace-state.cc -- State during tracing
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "trace-state.h"

#include "scene.h"

using namespace Snogray;

TraceState::TraceState (Scene &_scene, unsigned _depth)
  : horizon_hint (0), scene (_scene), depth (_depth)
{
  unsigned num_lights = _scene.num_lights ();

  shadow_hints = new Obj*[num_lights];
  for (unsigned i = 0; i < num_lights; i++)
    shadow_hints[i] = 0;

  for (unsigned i = 0; i < NUM_SUBTRACE_TYPES; i++)
    subtrace_states[i] = 0;
}

TraceState::~TraceState ()
{
  for (unsigned i = 0; i < NUM_SUBTRACE_TYPES; i++)
    delete subtrace_states[i];

  delete[] shadow_hints;
}

// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
