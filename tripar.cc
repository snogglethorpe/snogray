// tripar.cc -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
// placement-new with CONTEXT); otherwise return zero.
//
const Surface::IsecInfo *
Tripar::intersect (Ray &ray, RenderContext &context) const
{
  dist_t t, u, v;
  if (intersects (ray, t, u, v))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, this, u, v);
    }

  return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Tripar::IsecInfo::make_intersect (Trace &trace) const
{
  Pos point = ray.end ();

  dist_t e1_len = tripar->e1.length (), inv_e1_len = 1 / e1_len;

  // Calculate the normal and tangent vectors.
  //
  Vec norm = cross (tripar->e2, tripar->e1).unit ();
  Vec s = tripar->e1 * inv_e1_len;
  Vec t = cross (s, norm);

  // Normal frame.
  //
  Frame normal_frame (point, s, t, norm);

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  Vec oe2 = normal_frame.to (tripar->e2); // tripar->e2 in object space
  dist_t duds = inv_e1_len;
  dist_t dvds = oe2.x ? 1 / oe2.x : 0;
  dist_t dvdt = oe2.y ? 1 / oe2.y : 0;
  UV dTds (duds, dvds), dTdt (0, dvdt);

  Intersect isec (ray, trace, tripar, normal_frame, UV (u, v), dTds, dTdt);

  isec.no_self_shadowing = true;

  return isec;
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Tripar::shadow (const ShadowRay &ray, RenderContext &) const
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
