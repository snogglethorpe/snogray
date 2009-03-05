// trace-context.h -- Context in which tracing occurrs
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

#ifndef __TRACE_CONTEXT_H__
#define __TRACE_CONTEXT_H__

#include <memory>

#include "trace-stats.h"
#include "trace-params.h"
#include "space-builder.h"
#include "mempool.h"
#include "pool.h"
#include "isec-cache.h"


namespace snogray {


// Context in which tracing occurs.  This structure holds per-thread global
// information, such as caches and statistics, etc.
//
class TraceContext
{
public:

  TraceContext (const TraceParams &_params);
  ~TraceContext ();

  const TraceParams &params;

  // This mempool is reset every time control returns to the top level
  // of a trace tree, so should not be used for anything longer-lived
  // that that.
  //
  Mempool mempool;

  // Pool of intersection caches.
  //
  // The reason that we keep these in a pool is that we need a separate
  // cache for each active search (and there be multiple active at once,
  // e.g. in the case of instancing), and constructing a a cache object
  // can be fairly expensive, so we don't want to allocate them on the
  // stack each tim.  Thus, we keep a pool of already-constructed cache
  // objects ready for use.
  //
  Pool<IsecCache> isec_cache_pool;

  TraceStats stats;

  std::auto_ptr<SpaceBuilderBuilder> space_builder_builder;
};


}

#endif /* __TRACE_CONTEXT_H__ */


// arch-tag: f3691316-99dd-4b9c-a405-8c816ba4e361
