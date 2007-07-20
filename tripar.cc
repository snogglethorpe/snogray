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


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
Surface::IsecInfo *
Tripar::intersect (Ray &ray, IsecCtx &isec_ctx) const
{
  dist_t u, v;
  if (tripar_intersect (v0, e1, e2, parallelogram, ray, u, v))
    return new (isec_ctx) IsecInfo (this);
  else
    return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Tripar::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  return
    Intersect (ray, tripar, ray.end (), cross (tripar->e1, tripar->e2), trace);
}

// Return true if this surface blocks RAY coming from ISEC.  This
// should be somewhat lighter-weight than Surface::intersect (and can
// handle special cases for some surface types).
//
bool
Tripar::shadows (const Ray &ray, const Intersect &) const
{
  return tripar_intersect (v0, e1, e2, parallelogram, ray);
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
