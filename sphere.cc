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

#include "sphere.h"
#include "sphere-isec.h"

#include "intersect.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
Surface::IsecInfo *
Sphere::intersect (Ray &ray, IsecCtx &isec_ctx) const
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
  return Intersect (ray, sphere, point, point - sphere->center, trace);
}

// Return true if this surface blocks RAY coming from ISEC.  This
// should be somewhat lighter-weight than Surface::intersect (and can
// handle special cases for some surface types).
//
bool
Sphere::shadows (const Ray &ray, const Intersect &) const
{
  dist_t t = sphere_intersect (center, radius, ray.origin, ray.dir, ray.t0);
  return (t > ray.t0 && t < ray.t1);
}

// Return a bounding box for this surface.
BBox
Sphere::bbox () const
{
  return BBox (Pos (center.x - radius, center.y - radius, center.z - radius),
	       Pos (center.x + radius, center.y + radius, center.z + radius));
}


// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
