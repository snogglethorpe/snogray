// trace-state.h -- State during tracing
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRACE_STATE_H__
#define __TRACE_STATE_H__

namespace Snogray {

class TraceState
{
  // Current depth of tracing.  1 == the main (camera/eye) ray.
  unsigned depth;

  // 
};

}

#endif /* __TRACE_STATE_H__ */

// arch-tag: 7ae04357-d63f-4119-9e79-a63d0e5a5e7f
