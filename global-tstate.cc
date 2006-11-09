// global-tstate.cc -- Global tracing state
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mis-illum.h"

#include "global-tstate.h"


using namespace Snogray;


GlobalTraceState::GlobalTraceState (const TraceParams &_params)
  : params (_params),
    illum_global_state (new MisIllumGlobalState)
{ }

// arch-tag: bacb1ae7-4350-408b-8746-cbcf77debc30
