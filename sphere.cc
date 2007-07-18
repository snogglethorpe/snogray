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


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
// to reflect the point of intersection, and return true; otherwise
// return false.  ISEC_PARAMS maybe used to pass information to a later
// call to Surface::intersect_info.
//
bool
Sphere::intersect (Ray &ray, IsecParams &) const
{
  dist_t t = sphere_intersect (center, radius, ray.origin, ray.dir, ray.t0);
  if (t > ray.t0 && t < ray.t1)
    {
      ray.t1 = t;
      return true;
    }
  else
    return false;
}

// Return an Intersect object containing details of the intersection of
// RAY with this surface; it is assumed that RAY does actually hit the
// surface, and RAY's maximum bound (Ray::t1) gives the exact point of
// intersection (the `intersect' method modifies RAY so that this is
// true).  ISEC_PARAMS contains other surface-specific parameters
// calculated by the previous call to Surface::intersects method.
//
Intersect
Sphere::intersect_info (const Ray &ray, const IsecParams &, Trace &trace)
  const
{
  Pos point = ray.end ();
  return Intersect (ray, this, point, point - center, trace);
}

// Return a bounding box for this surface.
BBox
Sphere::bbox () const
{
  return BBox (Pos (center.x - radius, center.y - radius, center.z - radius),
	       Pos (center.x + radius, center.y + radius, center.z + radius));
}


// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
