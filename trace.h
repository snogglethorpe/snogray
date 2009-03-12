// trace.h -- State during tracing
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

#ifndef __TRACE_H__
#define __TRACE_H__

#include "medium.h"


namespace snogray {


class TraceContext;
class TraceCache;


class Trace
{
public:

  enum Type {
    SPONTANEOUS,
    REFLECTION,
    REFRACTION_IN,		// entering a transparent surface
    REFRACTION_OUT,		// exiting it
    NUM_TRACE_TYPES
  };


  // Constructor for a camera/eye Trace.
  //
  Trace (TraceContext &_context, TraceCache &_root_cache);

  // Constructor for sub-traces
  //
  Trace (float branch_factor, Type type, const Medium &_medium,
	 const Trace &_source);

  // Searches back through the trace history to find the enclosing medium.
  //
  const Medium &enclosing_medium () const;

  // Return the depth of tracing at this trace.  1 == (camera/eye) ray.
  //
  unsigned depth () const
  {
    unsigned d = 1;
    for (const Trace *t = source; t; t = t->source)
      d++;
    return d;
  }


  // Source trace, or zero if this is the first.
  //
  const Trace *source;

  // Stuff that's only allocated once.  The context is generally
  // modifiable (e.g., for memory allocation or stats) even when a Trace
  // is const, so mark it as mutable.
  //
  mutable TraceContext &context;

  // What kind of trace this is
  //
  Type type;

  // This is a very rough guess at the number of paths will reach this
  // point in the rendering tree.  It is computed simply by multiplying
  // by the branching factor with each recursive trace (and so would
  // only be truly accurate if all paths reached the same recursion
  // depth).
  //
  float complexity;

  // The medium this trace is through.
  //
  const Medium &medium;

  // Downward tree of cached information at this trace point.  The cache
  // is generally modifiable even when a Trace is const, so mark it as
  // mutable.
  //
  mutable TraceCache &cache;

private:

  void _init ();
};

}


#endif /* __TRACE_H__ */


// arch-tag: 7ae04357-d63f-4119-9e79-a63d0e5a5e7f
