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

#include "intersect.h"
#include "shadow-ray.h"

#include "tripar.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Tripar::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  dist_t t, u, v;
  if (intersects (ray, t, u, v))
    {
      ray.t1 = t;
      return new (isec_ctx) IsecInfo (this);
    }

  return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Tripar::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();

  // Calculate the normal and tangent vectors.
  //
  Vec norm = cross (tripar->e1, tripar->e2).unit ();
  Vec s = tripar->e1.unit ();
  Vec t = cross (norm, s);

  Intersect isec (ray, tripar, Frame (point, s, t, norm), trace);

  isec.no_self_shadowing = true;

  return isec;
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Tripar::shadow (const ShadowRay &ray) const
{
  dist_t t, u, v;
  if (intersects (ray, t, u, v))
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}

// Return a bounding box for this surface.
//
BBox
Tripar::bbox () const
{
  BBox bbox (v0);
  bbox += v0 + e1;
  bbox += v0 + e2;
  if (parallelogram)
    bbox += v0 + e1 + e2;
  return bbox;
}


// arch-tag: 962df04e-4c0a-4754-ac1a-f506d4e77c4e
