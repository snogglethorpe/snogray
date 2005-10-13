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

// Constructor for root TraceState
//
TraceState::TraceState (Scene &_scene)
  : scene (_scene), parent (0), type (SPONTANEOUS),
    horizon_hint (0), depth (0), medium (0)
{
  _init ();
}

// Constructor for sub-traces
//
TraceState::TraceState (TraceType _type, TraceState *_parent)
  : scene (_parent->scene), parent (_parent), type (_type),
    horizon_hint (0), depth (_parent->depth + 1), medium (parent->medium)
{
  _init ();
}

void
TraceState::_init ()
{
  unsigned num_lights = scene.num_lights ();

  shadow_hints = new const Obj*[num_lights];
  for (unsigned i = 0; i < num_lights; i++)
    shadow_hints[i] = 0;

  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    subtrace_states[i] = 0;
}

TraceState::~TraceState ()
{
  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    delete subtrace_states[i];

  delete[] shadow_hints;
}

// Searches back through the trace history to find the enclosing medium.
//
const Medium *
TraceState::enclosing_medium ()
{
  const TraceState *ts = this;

  int enclosure_level = 0;

  while (enclosure_level >= 0 && ts)
    {
      if (ts->type == REFRACTION_IN)
	enclosure_level--;
      else if (ts->type == REFRACTION_OUT)
	enclosure_level++;

      ts = ts->parent;
    }

  return ts ? ts->medium : 0;
}

// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
