// trace.cc -- State during tracing
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "trace-context.h"

#include "trace.h"

using namespace snogray;

// Constructor for root Trace
//
Trace::Trace (TraceContext &_context)
  : parent (0), context (_context),
    type (SPONTANEOUS), horizon_hint (0),
    complexity (1), medium (0)
{
  _init ();
}

// Constructor for sub-traces
//
Trace::Trace (Type _type, Trace *_parent)
  : parent (_parent), context (_parent->context),
    type (_type), horizon_hint (0),
    complexity (1), medium (parent->medium)
{
  _init ();
}

void
Trace::_init ()
{
  unsigned num_lights = context.scene.num_lights ();

  shadow_hints = new const Surface*[num_lights];
  for (unsigned i = 0; i < num_lights; i++)
    shadow_hints[i] = 0;

  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    subtraces[i] = 0;
}

Trace::~Trace ()
{
  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    delete subtraces[i];

  delete[] shadow_hints;
}



// Searches back through the trace history to find the enclosing medium.
//
const Medium *
Trace::enclosing_medium ()
{
  const Trace *ts = this;

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
