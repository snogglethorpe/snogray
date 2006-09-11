// intersect.h -- Datatype for recording scene-ray intersection result
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __INTERSECT_H__
#define __INTERSECT_H__

#include "ray.h"
#include "color.h"
#include "trace.h"
#include "material.h"

namespace Snogray {

class Surface;
class Brdf;
class Material;

// This just packages up the result of a scene intersection search and
// some handy values calculated from it.  It is passed to rendering
// methods.
//
class Intersect
{
public:

  Intersect (const Ray &_ray, const Surface *_surface,
	     const Pos &_point, const Vec &_normal, bool _back,
	     Trace &_trace, const void *_smoothing_group = 0);

  // For surfaces with non-interpolated normals, we can calculate
  // whether it's a backface or not using the normal; they typically
  // also have a zero smoothing group, so we omit that parameter.
  //
  Intersect (const Ray &_ray, const Surface *_surface,
	     const Pos &_point, const Vec &_normal, Trace &_trace);

  // Calculate the outgoing radiance from this intersection.
  //
  Color render () const
  {
    return material.render (*this);
  }

  // Shadow LIGHT_RAY, which points to a light with (apparent) color
  // LIGHT_COLOR. and return the shadow color.  This is basically like
  // the `render' method, but calls the material's `shadow' method
  // instead of its `render' method.
  //
  // Note that this method is only used for `non-opaque' shadows --
  // opaque shadows (the most common kind) don't use it!
  //
  Color shadow (const Ray &light_ray, const Color &light_color,
		const Light &light)
    const
  {
    return material.shadow (*this, light_ray, light_color, light);
  }

  // Returns a pointer to the trace for a subtrace of the given
  // type (possibly creating a new one, if no such subtrace has yet been
  // encountered).
  //
  Trace &subtrace (Trace::Type type, const Medium *medium) const
  {
    return trace.subtrace (type, medium, surface);
  }
  // For sub-traces with no specified medium, propagate the current one.
  //
  Trace &subtrace (Trace::Type type) const
  {
    return trace.subtrace (type, surface);
  }

  // Iterate over every light, calculating its contribution the color of
  // ISEC.  BRDF is used to calculate the actual effect; COLOR is
  // the "base color"
  //
  Color illum () const;

  // Transform VEC from a "Z-normal" coordinate system (where the
  // intersection normal is (0,0,1)) to world coordinates.
  //
  Vec z_normal_to_world (const Vec &vec) const
  {
    return vec.to_basis (s, t, n);
  }

  // Transform VEC from world coordinates to the "Z-normal" coordinate
  // system (where the intersection normal is (0,0,1)).
  //
  Vec world_to_z_normal (const Vec &vec) const
  {
    return vec.from_basis (s, t, n);
  }

  // Returns the cosine of the angle between the surface normal and the
  // normalized vector VEC.
  //
  double cos_n (const Vec &vec) const
  {
    return min (dot (n, vec), 1.); // use min to clamp precision errors
  }

  // Returns the cosine of the angle between the viewing direction and the
  // normalized vector VEC.
  //
  double cos_v (const Vec &vec) const
  {
    return min (dot (v, vec), 1.); // use min to clamp precision errors
  }

  // Ray which intersected something; its endpoint is the point of intersection.
  //
  const Ray ray;

  // The surface which RAY intersected.  This should always be non-zero
  // (it's not a reference because all uses are as a pointer).
  //
  const Surface *surface;

  // Details of the intersection.
  //
  const Pos pos;		// Point where RAY intersects SURFACE

  // Normalized surface normal at POS.
  //
  const Vec n;

  // Vectors which are orthonormal with the surface normal N.
  //
  const Vec s, t;

  // A unit vector pointing towards the viewer; this is just -RAY.dir; many
  // algorithms use the outgoing formulation, so we provide it explicitly.
  //
  const Vec v;

  // (N dot V), aka cos(theta) where theta is the angle between N and V.
  //
  float nv;

  // True if RAY hit the back of SURFACE (relative to the normal).
  //
  const bool back;

  // Oft-used properties of SURFACE.
  //
  const Material &material;
  const Brdf &brdf;
  const Color &color;
  const void *smoothing_group;

  // Trace this intersection came from.
  //
  Trace &trace;
};

}

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
