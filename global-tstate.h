// global-tstate.h -- Global tracing state
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLOBAL_TSTATE_H__
#define __GLOBAL_TSTATE_H__

#include "trace-stats.h"
#include "trace-params.h"
#include "lsamples.h"

namespace Snogray {

class GlobalTraceState
{
public:

  GlobalTraceState (const TraceParams &_params) : params (_params) { }

  const TraceParams &params;

  TraceStats stats;

  // This is only used temporarily by Scene::illum, but we keep it around
  // permanently to avoid the overhead of memory allocation.
  //
  LightSamples light_samples;
};

}

#endif /* __GLOBAL_TSTATE_H__ */

// arch-tag: f3691316-99dd-4b9c-a405-8c816ba4e361
