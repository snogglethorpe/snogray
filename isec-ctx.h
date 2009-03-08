// isec-ctx.h -- Context information for intersection testing
//
//  Copyright (C) 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ISEC_CTX_H__
#define __ISEC_CTX_H__

#include "intersect.h"


namespace snogray {


class TraceContext;
class TraceCache;


// A container for various context information used for intersection test.
//
class IsecCtx
{
public:

  IsecCtx (TraceContext &_context, TraceCache &_cache)
    : context (_context), cache (_cache)
  { }
  IsecCtx (const Intersect &isec)
    : context (isec.context), cache (isec.trace.cache)
  { }

  // Global tracing context.
  //
  TraceContext &context;

  // Information cached at this point in the trace.
  //
  TraceCache &cache;
};


}


// The user can use this via placement new: "new (ISEC_CTX) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
// All memory allocated from an IsecCtx object is automatically
// freed at some appropriate point.
//
inline void *
operator new (size_t size, const snogray::IsecCtx &isec_ctx)
{
  return operator new (size, isec_ctx.context);
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
inline void
operator delete (void *mem, const snogray::IsecCtx &isec_ctx)
{
  operator delete (mem, isec_ctx.context);
}


#endif // __ISEC_CTX_H__
