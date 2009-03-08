// intersect.h -- Datatype for recording scene-ray intersection result
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

#ifndef __INTERSECT_H__
#define __INTERSECT_H__

#include "uv.h"
#include "ray.h"
#include "frame.h"
#include "color.h"
#include "trace.h"
#include "trace-context.h"
#include "tex-coords.h"


namespace snogray {

class Surface;
class Material;
class Brdf;


// This just packages up the result of a scene intersection search and
// some handy values calculated from it.  It is passed to rendering
// methods.
//
class Intersect
{
public:

  Intersect (const Ray &_ray, const Surface *_surface,
	     const Frame &_normal_frame, const UV &_tex_coords,
	     const UV &dTds, const UV &dTdt, Trace &_trace);
  Intersect (const Ray &_ray, const Surface *_surface,
	     const Frame &_normal_frame, const Frame &_geom_frame,
	     const UV &_tex_coords,
	     const UV &dTds, const UV &dTdt, Trace &_trace);

  ~Intersect ();

  // Returns a pointer to the trace for a subtrace of the given
  // type (possibly creating a new one, if no such subtrace has yet been
  // encountered).
  //
  Trace &subtrace (float branch_factor, Trace::Type type, const Medium *medium)
    const
  {
    return trace.subtrace (branch_factor, type, medium);
  }
  // For sub-traces with no specified medium, propagate the current one.
  //
  Trace &subtrace (float branch_factor, Trace::Type type) const
  {
    return trace.subtrace (branch_factor, type);
  }

  // Returns the cosine of the angle between the surface normal and VEC.
  // VEC must be in this intersection's normal frame, and must be
  // normalized.
  //
  dist_t cos_n (const Vec &vec) const
  {
    // As VEC is normalized, cos(theta) is (N dot VEC), and as VEC is in
    // the normal frame, where N = (0,0,1), (N dot VEC) is just VEC.z.
    //
    return vec.z;
  }

  // Returns the cosine of the angle between the geometric surface
  // normal and VEC.  VEC must be in this intersection's normal frame,
  // and must be normalized.
  //
  // This may be different from Intersect::cos_n when the surface normal
  // has been perturbed due to bump-mapping or normal interpolation in
  // meshes.
  //
  dist_t cos_geom_n (const Vec &vec) const
  {
    // As VEC is normalized, cos(theta) is (GEOM_N dot VEC).
    //
    return dot (vec, geom_n);
  }

  // Returns the cosine of the angle between the viewing direction and VEC.
  // VEC must be in this intersection's normal frame, and must be
  // normalized.
  //
  dist_t cos_v (const Vec &vec) const
  {
    return min (dot (v, vec), dist_t (1)); // use min to clamp precision errors
  }

  // Return a mempool for intersection-related allocations.
  //
  Mempool &mempool () const { return context.mempool; }

  // Ray which intersected something; its endpoint is the point of intersection.
  //
  Ray ray;

  // The surface which RAY intersected.  This should always be non-zero
  // (it's not a reference because all uses are as a pointer).
  //
  const Surface *surface;

  // A frame of reference corresponding to the surface-normal.  Most
  // lighting calculations are done in this frame of reference.
  //
  // The position of intersection and the surface normal in world
  // space are "normal_frame.origin" and "normal_frame.z"
  // respectively; "normal_frame.x" and "normal_frame.y" are
  // orthogonal surface tangent vectors.
  //
  // This frame is kept in a "flipped" state, so that the eye-vector V
  // is in the same hemisphere as the normal.
  //
  Frame normal_frame;

  // A frame of reference roughly similar to NORMAL_FRAME, but
  // corresponding to the true surface geometry, with no normal
  // perturbations (by bump-mapping etc) applied.
  //
  // Unlike NORMAL_FRAME, GEOM_FRAME is not "flipped" to place the
  // normal is in the same hemisphere as the eye-vector.
  //
  Frame geom_frame;

  // The eye vector, a unit vector pointing towards the viewer, in the
  // normal frame.
  //
  Vec v;

  // The geometric surface normal (corresponding to the true surface
  // geometry, with no normal perturbations applied), in the normal
  // frame.
  //
  // Unlike GEOM_FRAME, GEOM_N is flipped so that it is always in the
  // same hemisphere as the lighting normal (i.e., GEOM_N.z is always
  // positive).
  //
  Vec geom_n;

  // True if RAY hit the back of SURFACE (relative to the normal).
  //
  bool back;

  // Material of the surface at this intersection.
  //
  const Material *material;

  // BRDF used at this intersection.
  //
  const Brdf *brdf;

  // Oft-used properties of SURFACE.
  //
  const void *smoothing_group;

  // If true, SURFACE cannot shadow itself (for non-planar objects, this
  // applies to the current side of SURFACE -- the one corresponding to
  // the normal N).  This is only used for optimization.
  //
  bool no_self_shadowing;

  // Texture coordinates for this intersection.
  //
  TexCoords tex_coords;

  // Trace this intersection came from.
  //
  Trace &trace;

  // Global tracing context.
  //
  TraceContext &context;

private:

  // Finish initialization.  This method is called by all constructors.
  //
  void finish_init (const UV &dTds, const UV &dTdt);
};


} // namespace snogray


// The user can use this via placement new: "new (ISEC) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
// All memory allocated from an intersection is automatically freed at
// some appropriate point, and should not be used after the intersection
// is destroyed.
//
inline void *operator new (size_t size, const snogray::Intersect &isec)
{
  return operator new (size, isec.context);
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
inline void operator delete (void *mem, const snogray::Intersect &isec)
{
  operator delete (mem, isec.context);
}


#endif // __INTERSECT_H__


// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
