// global-tstate.h -- Global tracing state
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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


namespace snogray {

class IllumGlobalState;


class GlobalTraceState
{
public:

  GlobalTraceState (const TraceParams &_params);

  const TraceParams &params;

  IllumGlobalState *illum_global_state;

  TraceStats stats;
};


}

#endif /* __GLOBAL_TSTATE_H__ */


// arch-tag: f3691316-99dd-4b9c-a405-8c816ba4e361
