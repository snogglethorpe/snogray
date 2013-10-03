// intersect.h -- Datatype for recording scene-ray intersection result
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_INTERSECT_H
#define SNOGRAY_INTERSECT_H

#include "util/ref.h"
#include "geometry/uv.h"
#include "geometry/ray.h"
#include "geometry/frame.h"
#include "material/bsdf.h"
#include "color/color.h"
#include "render/render-context.h"
#include "texture/tex-coords.h"


namespace snogray {

class Surface;
class Material;
class Media;


// This just packages up the result of a scene intersection search and
// some handy values calculated from it.  It is passed to rendering
// methods.
//
class Intersect
{
public:

  // Note that the RAY may not be in the world coordinate system, if
  // the intersected surface is inside an instance or something.
  //
  Intersect (const Ray &ray, const Media &_media, RenderContext &context,
	     const Material &_material,
	     const Frame &_normal_frame,
	     const UV &_tex_coords_uv, const UV &dTds, const UV &dTdt);
  Intersect (const Ray &ray, const Media &_media, RenderContext &context,
	     const Material &_material,
	     const Frame &_normal_frame, const Frame &_geom_frame,
	     const UV &_tex_coords_uv, const UV &dTds, const UV &dTdt);

  // Copy-constructor.
  //
  Intersect (const Intersect &isec);

  // Return directly-emitted radiance from this intersection.
  //
  Color Le () const;

  // Return the cosine of the angle between the surface normal and VEC.
  // VEC must be in this intersection's normal frame, and must be
  // normalized.
  //
  // If VEC is a null vector, containing all zeroes, then return 0.
  //
  float cos_n (const Vec &vec) const
  {
    // As VEC is normalized, cos(theta) is (N dot VEC), and as VEC is in
    // the normal frame, where N = (0,0,1), (N dot VEC) is just VEC.z.
    //
    return float (vec.z);
  }

  // Return the cosine of the angle between the geometric surface normal
  // and VEC.  VEC must be in this intersection's normal frame, and must
  // be normalized.
  //
  // This may be different from Intersect::cos_n when the surface normal
  // has been perturbed due to bump-mapping or normal interpolation in
  // meshes.
  //
  // If VEC is a null vector, containing all zeroes, then return 0.
  //
  float cos_geom_n (const Vec &vec) const
  {
    // As VEC is normalized, cos(theta) is (GEOM_N dot VEC).
    //
    return cos_angle (vec, geom_n);
  }

  // Return the cosine of the angle between the viewing direction and
  // VEC.  VEC must be in this intersection's normal frame, and must be
  // normalized.
  //
  // If VEC is a null vector, containing all zeroes, then return 0.
  //
  float cos_v (const Vec &vec) const
  {
    return min (cos_angle (v, vec), 1.f); // use min to clamp precision errors
  }

  // Return a mempool for intersection-related allocations.
  //
  Mempool &mempool () const { return context.mempool; }

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

  // True if the source ray hit the back of the surface (relative to the
  // normal).
  //
  bool back;

  // Material of the surface at this intersection.
  //
  const Material &material;

  // BSDF used at this intersection.
  //
  const Bsdf *bsdf;

  // Media surrounding this intersection; the innermost is that through
  // which RAY came.
  //
  const Media &media;

  // Global tracing context.
  //
  RenderContext &context;

private:

  // Finish initialization.  This method is called by all constructors.
  //
  void finish_init (const Ray &ray, const UV &dTds, const UV &dTdt);

  // Surface UV texture coordinates for this intersection.  This field
  // is private because these are the "raw" texture-coordinates, which
  // are not correct in all contexts.
  //
  UV tex_coords_uv;
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


#endif // SNOGRAY_INTERSECT_H


// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
