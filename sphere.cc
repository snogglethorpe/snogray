// sphere.cc -- Sphere surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "shadow-ray.h"
#include "sphere-isec.h"

#include "sphere.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Sphere::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  dist_t t = sphere_intersect (center, radius, ray.origin, ray.dir, ray.t0);
  if (t > ray.t0 && t < ray.t1)
    {
      ray.t1 = t;
      return new (isec_ctx) IsecInfo (this);
    }
  else
    return false;
}

// Create an Intersect object for this intersection.
//
Intersect
Sphere::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();
  Intersect isec (ray, sphere, point, point - sphere->center, trace);

  isec.no_self_shadowing = !isec.back;

  return isec;
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType Sphere::shadow (const ShadowRay &ray) const
{
  dist_t t = sphere_intersect (center, radius, ray.origin, ray.dir, ray.t0);
  if (t > ray.t0 && t < ray.t1)
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}

// Return a bounding box for this surface.
BBox
Sphere::bbox () const
{
  return BBox (Pos (center.x - radius, center.y - radius, center.z - radius),
	       Pos (center.x + radius, center.y + radius, center.z + radius));
}


// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
