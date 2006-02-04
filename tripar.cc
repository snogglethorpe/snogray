// tripar.cc -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "tripar.h"
#include "tripar-isec.h"

#include "intersect.h"

using namespace Snogray;

// Return the distance from RAY's origin to the closest intersection
// of this surface with RAY, or 0 if there is none.  RAY is considered
// to be unbounded.
//
// If intersection succeeds, then ISEC_PARAMS is updated with other
// (surface-specific) intersection parameters calculated.
//
// NUM is which intersection to return, for non-flat surfaces that may
// have multiple intersections -- 0 for the first, 1 for the 2nd, etc
// (flat surfaces will return failure for anything except 0).
//
dist_t
Tripar::intersection_distance (const Ray &ray, IsecParams &isec_params,
			       unsigned num)
  const
{
  if (num != 0)
    return 0;

  return tripar_intersect (v0, e1, e2, parallelogram, ray.origin, ray.dir,
			   isec_params.u, isec_params.v);
}

Intersect
Tripar::intersect_info (const Ray &ray, const IsecParams &isec_params)
  const
{
  return Intersect (ray, this, ray.end (), (e1.cross (e2)).unit ());
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
