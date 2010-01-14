// render-context.h --  "semi-global" information used during rendering
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__

#include "trace-stats.h"
#include "trace-params.h"
#include "space-builder.h"
#include "mempool.h"
#include "medium.h"
#include "pool.h"
#include "isec-cache.h"
#include "unique-ptr.h"


namespace snogray {


// Context in which tracing occurs.  This structure holds per-thread global
// information, such as caches and statistics, etc.
//
class RenderContext
{
public:

  RenderContext (const Scene &_scene, const TraceParams &_params);
  ~RenderContext ();

  // Scene being rendered.
  //
  const Scene &scene;

  const TraceParams &params;

  // Medium assumed to surround all objects.
  //
  Medium default_medium;

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

  UniquePtr<SpaceBuilderFactory> space_builder_factory;
};


}


// The user can use this via placement new: "new (CONTEXT) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
// All memory allocated from a context object is automatically freed at
// some appropriate point, and should not be used after a trace has
// completed (i.e., this is temporary storage).
//
inline void *operator new (size_t size, snogray::RenderContext &context)
{
  return operator new (size, context.mempool);
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
inline void operator delete (void *mem, snogray::RenderContext &context)
{
  operator delete (mem, context.mempool);
}


#endif /* __RENDER_CONTEXT_H__ */


// arch-tag: f3691316-99dd-4b9c-a405-8c816ba4e361
