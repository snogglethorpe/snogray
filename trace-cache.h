// trace-cache.h -- Cache for data that persists between traces
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

#ifndef __TRACE_CACHE_H__
#define __TRACE_CACHE_H__

#include "trace.h"


namespace snogray {

class Surface;


// A cache for data that we try to maintain so that future "nearby" traces
// can use it.  This structure holds a single node.
//
// TraceCache objects form a downward tree, consistent of traces from
// camera to light(s); it's a tree because at each node, we sometimes
// maintain independent caches for alternative rendering situations, e.g.,
// refraction verusus reflection.  Note that this is in contrast to the
// main Trace objects, which form an upward trace from light to camera.
//
// When constructing new traces, we can follow the trace node cache tree and
// attach the caches to the nodes in the trace.
//
class TraceCache
{
public:

  TraceCache (const TraceContext &context);
  ~TraceCache ();

  // Returns a pointer to a cache for a subtrace of the given type
  // (possibly creating a new one, if no such subtrace has yet been
  // encountered).
  //
  TraceCache &sub_cache (Trace::Type type, const TraceContext &context)
  {
    TraceCache *sub = sub_caches[type];

    if (! sub)
      {
	sub = new TraceCache (context);
	sub_caches[type] = sub;
      }

    return *sub;
  }


  // If non-zero, the last surface we found as the closest intersection.
  // When we do a new trace, we first test that surface for intersection;
  // if it intersects, it is used to set the initial ray horizon, which
  // can drastically reduce the search space by excluding all further
  // surfaces.
  //
  const Surface *horizon_hint;

  // An array, indexed by "light number".  Each non-zero entry is an
  // surface previously found to shadow the given light.  Because nearby
  // points are often shadowed from a given light by the same surface(s),
  // testing these surfaces often yields a shadow surface without searching.
  //
  const Surface **shadow_hints;

  // Possible branches in the cache tree, with one potential branch for
  // each trace type.
  //
  TraceCache *sub_caches[Trace::NUM_TRACE_TYPES];
};


}


#endif // __TRACE_CACHE_H__
