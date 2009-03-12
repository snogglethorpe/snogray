// trace.cc -- Tracing path
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

#include "trace-context.h"
#include "trace-cache.h"

#include "trace.h"

using namespace snogray;


// Constructor for root Trace
//
Trace::Trace (TraceContext &_context, TraceCache &_root_cache)
  : source (0), context (_context), type (SPONTANEOUS),
    complexity (1), medium (_context.default_medium),
    cache (_root_cache)
{ }

// Constructor for sub-traces
//
Trace::Trace (float branch_factor, Type _type, const Medium &_medium,
	      const Trace &_source)
  : source (&_source), context (_source.context), type (_type),
    complexity (_source.complexity * branch_factor), medium (_medium),
    cache (source->cache.sub_cache (type, context))
{ }

// Searches back through the trace history to find the enclosing medium.
//
const Medium &
Trace::enclosing_medium () const
{
  const Trace *ts = this;

  int enclosure_level = 0;

  while (enclosure_level >= 0 && ts)
    {
      if (ts->type == REFRACTION_IN)
	enclosure_level--;
      else if (ts->type == REFRACTION_OUT)
	enclosure_level++;

      ts = ts->source;
    }

  return ts ? ts->medium : context.default_medium;
}


// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
