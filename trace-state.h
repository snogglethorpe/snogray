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

#include "color.h"

namespace Snogray {

class Scene;
class Obj;
class Ray;

class TraceState
{
public:

  enum TraceType {
    TRACE_SPONTANEOUS,
    TRACE_REFLECTION,
    TRACE_REFRACTION_IN,
    TRACE_REFRACTION_OUT,
    NUM_TRACE_TYPES
  };

  TraceState (Scene &_scene);
  TraceState (TraceType _type, TraceState *_parent);
  ~TraceState ();

  // Returns a pointer to the trace-state for a subtrace of the given
  // type (possibly creating a new one, if no such subtrace has yet been
  // encountered).
  //
  TraceState &subtrace_state (TraceType type, float _ior)
  {
    TraceState *sub = subtrace_states[type];

    if (! sub)
      {
	sub = new TraceState (type, this);
	subtrace_states[type] = sub;
      }

    sub->ior = _ior;		// make sure it's up to date

    return *sub;
  }

  // For sub-traces with no specified index of refraction, always
  // propagate the current one.
  //
  TraceState &subtrace_state (TraceType type)
  {
    return subtrace_state (type, ior);
  }

  // Convenience method that just calls the scene's render method using
  // this as the trace  state.
  //
  Color render (const Ray &ray, const Obj *origin = 0);

  // Searches back through the trace history to find the enclosing
  // medium, and returns its trace-state.  If there is no enclosing
  // medium returns 0.
  //
  const TraceState *enclosing_medium_state ();

  // Searches back through the trace history to find the enclosing
  // medium, and returns its index of refraction.
  //
  float enclosing_medium_ior ()
  {
    const TraceState *ts = enclosing_medium_state  ();
    return ts ? ts->ior : 1;
  }


  Scene &scene;

  // Parent state
  //
  TraceState *parent;

  // What kind of trace this is
  //
  TraceType type;

  // If non-zero, the last object we found as the closest intersection.
  // When we do a new trace, we first test that object for intersection;
  // if it intersects, it is used to set the initial ray horizon, which
  // can drastically reduce the search space by excluding all further
  // objects.
  //
  const Obj *horizon_hint;

  // An array, indexed by "light number".  Each non-zero entry is an
  // object previously found to shadow the given light.  Because nearby
  // points are often shadowed from a given light by the same object(s),
  // testing these objects often yields a shadow object without searching.
  //
  const Obj **shadow_hints;

  // Trace-states for various possible sub-traces of this trace (or zero
  // when a given subtrace-type hasn't yet been encountered at this
  // level).  Trace-states form a tree with the primary trace as the
  // root, and various possible recursive traces as children.
  //
  TraceState *subtrace_states[NUM_TRACE_TYPES];

  // Depth of tracing at this trace-state.  1 == the main (camera/eye) ray.
  //
  unsigned depth;

  // The index of refraction of the current material (1 == air)
  //
  float ior;

private:

  void _init ();
};

}

#endif /* __TRACE_STATE_H__ */

// arch-tag: 7ae04357-d63f-4119-9e79-a63d0e5a5e7f
