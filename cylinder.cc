// cylinder.cc -- Cylindrical surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "quadratic-isec.h"
#include "intersect.h"
#include "shadow-ray.h"

#include "cylinder.h"


using namespace snogray;


// Return the point of intersection of RAY with a radius 1 cylinder of
// height 2, centered at the origin.  If RAY doesn't intersect, return
// zero, otherwise the return value T is the parametric distance from
// RAY.origin; to calculate the actual intersection point, use
// RAY.origin + T * RAY.dir.
//
static dist_t
cylinder_intersect (Ray &ray)
{
  const Pos &o = ray.origin;
  const Vec &d = ray.dir;

  dist_t a = d.x * d.x + d.y * d.y;
  dist_t b = 2 * (d.x * o.x + d.y * o.y);
  dist_t c = o.x * o.x + o.y * o.y - 1 /* radius */;

  // Compute intersection point.
  //
  dist_t t0, t1;
  if (quadratic_isec (a, b, c, t0, t1))
    {
      if (t0 > ray.t0 && t0 < ray.t1)
	{
	  coord_t z = ray.origin.z + t0 * ray.dir.z;
	  if (z >= -1 && z <= 1)
	    return t0;
	}
      if (t1 > ray.t0 && t1 < ray.t1)
	{
	  coord_t z = ray.origin.z + t1 * ray.dir.z;
	  if (z >= -1 && z <= 1)
	    return t1;
	}
    }

  return 0;
}



// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Cylinder::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  Ray oray = world_to_local (ray);
  dist_t t = cylinder_intersect (oray);

  if (t != 0)
    {
      ray.t1 = t;

      coord_t ix = oray.origin.x + t * oray.dir.x;
      coord_t iy = oray.origin.y + t * oray.dir.y;

      return new (isec_ctx) IsecInfo (this, ix, iy);
    }
  else
    return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Cylinder::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();
  Vec onorm = Vec (isec_x, isec_y, 0);
  Vec norm = cylinder->normal_to_world (onorm);
  return Intersect (ray, cylinder, point, norm, trace);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Cylinder::shadow (const ShadowRay &sray) const
{
  Ray oray = world_to_local (sray.as_ray ());

  if (cylinder_intersect (oray) != 0)
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}


// arch-tag: 1a4758de-f640-4ea6-abf2-2626070847e5
