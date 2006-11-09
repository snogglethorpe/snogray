// intersect.h -- Datatype for recording surface-ray intersection results
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface.h"
#include "material.h"
#include "brdf.h"
#include "trace.h"
#include "scene.h"
#include "global-tstate.h"

#include "intersect.h"

using namespace Snogray;



Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Pos &_pos, const Vec &_normal, bool _back,
		      Trace &_trace, const void *_smoothing_group)
  : ray (_ray), surface (_surface), pos (_pos),
    n ((_back ? -_normal : _normal).unit ()),
    s (n.perpendicular ().unit ()), t (cross (s, n).unit ()),
    v (-_ray.dir), nv (dot (n, v)), back (_back),
    material (*_surface->material),
    brdf (material.brdf), color (material.color),
    smoothing_group (_smoothing_group), trace (_trace)
{ }

// For surfaces with non-interpolated normals, we can calculate
// whether it's a backface or not using the normal; they typically
// also have a zero smoothing group, so we omit that parameter.
//
Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Pos &_pos, const Vec &_normal, Trace &_trace)
  : ray (_ray), surface (_surface),
    pos (_pos), n (_normal.unit ()),
    s (n.perpendicular ().unit ()), t (cross (s, n).unit ()),
    v (-_ray.dir), nv (dot (n, v)),
    back (nv < 0),
    material (*_surface->material),
    brdf (material.brdf), color (material.color),
    smoothing_group (0), trace (_trace)
{
  // We want to flip the sign on `normal' if `back' is true, but we've
  // declared `normal' const to avoid anybody mucking with it...
  //
  if (back)
    {
      const_cast<Vec&> (n) = -n;
      const_cast<float&> (nv) = -nv;
    }
}



Color
Intersect::illum () const
{
  trace.global.stats.illum_calls++;
  return trace.illuminator().illum (*this);
}  


// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
