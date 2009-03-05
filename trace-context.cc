// trace-context.cc -- Trace tracing state
//
//  Copyright (C) 2006, 2007, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mis-illum.h"
#include "octree.h"

#include "trace-context.h"


using namespace snogray;


TraceContext::TraceContext (const TraceParams &_params)
  : params (_params),
    space_builder_builder (new Octree::BuilderBuilder) // hardwired for now
{ }

TraceContext::~TraceContext ()
{
}


// arch-tag: bacb1ae7-4350-408b-8746-cbcf77debc30
