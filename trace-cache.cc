// trace-cache.cc -- Cache for data that persists between traces
//
//  Copyright (C) 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "trace.h"
#include "trace-context.h"

#include "trace-cache.h"


using namespace snogray;


TraceCache::TraceCache (const TraceContext &context)
  : horizon_hint (0)
{
  unsigned num_lights = context.scene.num_lights ();

  shadow_hints = new const Surface*[num_lights];
  for (unsigned i = 0; i < num_lights; i++)
    shadow_hints[i] = 0;

  for (unsigned i = 0; i < Trace::NUM_TRACE_TYPES; i++)
    sub_caches[i] = 0;
}

TraceCache::~TraceCache ()
{
  for (unsigned i = 0; i < Trace::NUM_TRACE_TYPES; i++)
    delete sub_caches[i];

  delete[] shadow_hints;
}
