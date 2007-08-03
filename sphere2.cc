// sphere2.cc -- Alternative sphere surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "sphere-isec.h"
#include "intersect.h"
#include "shadow-ray.h"

#include "sphere2.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
Surface::IsecInfo *
Sphere2::intersect (Ray &ray, IsecCtx &isec_ctx) const
{
  Ray oray = world_to_local (ray);
  
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);
  if (t > oray.t0 && t < oray.t1)
    {
      ray.t1 = t;
      return new (isec_ctx) IsecInfo (this, Vec (oray.extension (t)));
    }
  else
    return false;
}

// Create an Intersect object for this intersection.
//
Intersect
Sphere2::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();
  Vec norm = sphere->normal_to_world (onorm);
  return Intersect (ray, sphere, point, norm, trace);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Sphere2::shadow (const ShadowRay &sray) const
{
  Ray oray = world_to_local (sray.as_ray ());
  dist_t t = sphere_intersect (dist_t(1), Vec (oray.origin), oray.dir, oray.t0);

  if (t > oray.t0 && t < oray.t1)
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}


// arch-tag: 4eacb938-b015-4a82-936b-effbdc72eab5
