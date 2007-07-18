// tripar.cc -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "tripar.h"
#include "tripar-isec.h"

#include "intersect.h"

using namespace snogray;

// If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
// to reflect the point of intersection, and return true; otherwise
// return false.  ISEC_PARAMS maybe used to pass information to a later
// call to Surface::intersect_info.
//
bool
Tripar::intersect (Ray &ray, IsecParams &isec_params) const
{
  return tripar_intersect (v0, e1, e2, parallelogram, ray,
			   isec_params.u, isec_params.v);
}

// Return an Intersect object containing details of the intersection of RAY
// with this surface; it is assumed that RAY does actually hit the surface,
// and RAY's maximum bound (Ray::t1) gives the exact point of intersection
// (the `intersect' method modifies RAY so that this is true).  ISEC_PARAMS
// contains other surface-specific parameters calculated by the previous
// call to Surface::intersects method.
//
Intersect
Tripar::intersect_info (const Ray &ray, const IsecParams &, Trace &trace)
  const
{
  return Intersect (ray, this, ray.end (), cross (e1, e2), trace);
}

// Return a bounding box for this surface.
BBox
Tripar::bbox () const
{
  BBox bbox (v0);
  bbox.include (v0 + e1);
  bbox.include (v0 + e2);
  if (parallelogram)
    bbox.include (v0 + e1 + e2);
  return bbox;
}

// arch-tag: 962df04e-4c0a-4754-ac1a-f506d4e77c4e
