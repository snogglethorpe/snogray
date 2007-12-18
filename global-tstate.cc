// global-tstate.cc -- Global tracing state
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
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

#include "global-tstate.h"


using namespace snogray;


GlobalTraceState::GlobalTraceState (const TraceParams &_params)
  : params (_params),
    illum_global_state (new MisIllumGlobalState),      // hardwired for now
    space_builder_builder (new Octree::BuilderBuilder) // hardwired for now
{ }

GlobalTraceState::~GlobalTraceState ()
{
  delete illum_global_state;
}


// arch-tag: bacb1ae7-4350-408b-8746-cbcf77debc30
