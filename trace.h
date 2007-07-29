// trace.h -- State during tracing
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRACE_H__
#define __TRACE_H__

#include "ray.h"
#include "color.h"
#include "medium.h"
#include "material.h"
#include "illum.h"
#include "isec-cache.h"
#include "global-tstate.h"


namespace snogray {


class Surface;
class Brdf;
class Light;
class Scene;
class Illum;
class Intersect;
class ShadowRay;
class GlobalTraceState;


class Trace
{
public:

  enum Type {
    SPONTANEOUS,
    REFLECTION,
    REFRACTION_IN,		// entering a transparent surface
    REFRACTION_OUT,		// exiting it
    TRANSMISSION,		// transmission through a thin surface
    SHADOW,			// only used for non-opaque shadows
    SHADOW_REFR_IN,
    SHADOW_REFR_OUT,
    NUM_TRACE_TYPES
  };

  Trace (const Scene &_scene, GlobalTraceState &_global);
  Trace (Type _type, Trace *_parent);
  ~Trace ();

  // Returns a pointer to the trace for a subtrace of the given
  // type (possibly creating a new one, if no such subtrace has yet been
  // encountered).
  //
  Trace &subtrace (Type type, const Medium *_medium, const Surface *_origin)
  {
    Trace *sub = subtraces[type];

    if (! sub)
      {
	sub = new Trace (type, this);
	subtraces[type] = sub;
      }

    // make sure fields are up-to-date
    //
    sub->origin = _origin;
    sub->medium = _medium;

    return *sub;
  }

  // For sub-traces with no specified medium, propagate the current one.
  //
  Trace &subtrace (Type type, const Surface *_origin)
  {
    return subtrace (type, medium, _origin);
  }

  // Calculate the color perceived by looking along RAY.  This is the
  // basic ray-tracing method.
  //
  Color render (const Ray &ray);

  // Shadow LIGHT_RAY, which points to a light with (apparent) color
  // LIGHT_COLOR. and return the shadow color.  This is basically like
  // the `render' method, but calls the material's `shadow' method
  // instead of its `render' method.
  //
  // Note that this method is only used for `non-opaque' shadows --
  // opaque shadows (the most common kind) don't use it!
  //
  Color shadow (const Ray &light_ray, const Color &light_color,
		const Light &light);

  // The following are convenience methods that just call the equivalent
  // method in the scene.
  //
  Material::ShadowType shadow (const ShadowRay &ray);

  // Searches back through the trace history to find the enclosing medium.
  //
  const Medium *enclosing_medium ();

  // Return the local illumination object for this trace.
  //
  Illum &illuminator ()
  {
    if (! _illum)
      _illum = global.illum_global_state->get_illum (*this);
    return *_illum;
  }


  const Scene &scene;

  // Parent state
  //
  Trace *parent;

  // Stuff that's only allocated once.
  //
  GlobalTraceState &global;

  // What kind of trace this is
  //
  Type type;

  // The surface this trace originated from (or zero for spontaneous)
  //
  const Surface *origin;

  // A cache of "negative" intersections.
  //
  IsecCache negative_isec_cache;

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

  // traces for various possible sub-traces of this trace (or zero
  // when a given subtrace-type hasn't yet been encountered at this
  // level).  traces form a tree with the primary trace as the
  // root, and various possible recursive traces as children.
  //
  Trace *subtraces[NUM_TRACE_TYPES];

  // Depth of tracing at this trace.  1 == the main (camera/eye) ray.
  //
  unsigned depth;

  // The medium this trace is through.  Zero means "air".
  //
  const Medium *medium;

private:

  // Illuminator for intersections.
  //
  Illum *_illum;

  void _init ();
};

}


// The user can use this via placement new: "new (TRACE) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
// All memory allocated from a trace object is automatically freed at some
// appropriate point, and should not be used after the trace has completed.
//
inline void *operator new (size_t size, snogray::Trace &trace)
{
  return operator new (size, trace.global.mempool);
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
inline void operator delete (void *mem, snogray::Trace &trace)
{
  operator delete (mem, trace.global.mempool);
}


#endif /* __TRACE_H__ */


// arch-tag: 7ae04357-d63f-4119-9e79-a63d0e5a5e7f
