// render-context.h --  "semi-global" information used during rendering
//
//  Copyright (C) 2005-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RENDER_CONTEXT_H
#define SNOGRAY_RENDER_CONTEXT_H

#include "util/unique-ptr.h"
#include "util/random.h"
#include "util/mempool.h"
#include "util/pool.h"
#include "material/medium.h"
#include "sample-set.h"
#include "surface-integ.h"
#include "volume-integ.h"
#include "render-stats.h"
#include "render-params.h"


namespace snogray {

class GlobalRenderState;
class IsecCache;


// Context in which tracing occurs.  This structure holds per-thread global
// information, such as caches and statistics, etc.
//
class RenderContext
{
public:

  RenderContext (const GlobalRenderState &global_state);
  ~RenderContext ();

  // Scene being rendered.
  //
  const Scene &scene;

  // Medium assumed to surround all objects.
  //
  Medium default_medium;

  // This mempool is reset every time control returns to the top level
  // of a media tree, so should not be used for anything longer-lived
  // that that.
  //
  Mempool mempool;

  // SampleSet used to hold samples for rendering each pixel.
  // The actual samples are regenerated for each pixel, but the
  // sample-set object also holds a set of "channels", which persist.
  //
  SampleSet samples;

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

  RenderStats stats;

  // Random number generator.  This is a callable object.
  //
  Random random;

  // Global state shared by all render-contexts.
  //
  const GlobalRenderState &global_state;

  // Commonly used rendering parameters.
  //
  const RenderParams params;

  // Surface integrator.  This should be one of the last fields, so it
  // will be initialized after other fields -- the integrator creation
  // method is passed a reference to the RenderContext object, so we
  // want as much RenderContext state as possible to be valid at that
  // point.
  //
  UniquePtr<SurfaceInteg> surface_integ;

  // Volume integrator.  This should be one of the last fields, so it
  // will be initialized after other fields -- the integrator creation
  // method is passed a reference to the RenderContext object, so we
  // want as much RenderContext state as possible to be valid at that
  // point.
  //
  UniquePtr<VolumeInteg> volume_integ;
};


}


// The user can use this via placement new: "new (CONTEXT) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
// All memory allocated from a context object is automatically freed at
// some appropriate point, and should not be used after a media has
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


#endif /* SNOGRAY_RENDER_CONTEXT_H */


// arch-tag: f3691316-99dd-4b9c-a405-8c816ba4e361
