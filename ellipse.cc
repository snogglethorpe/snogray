// ellipse.cc -- Ellipse surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
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

#include "ellipse.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Ellipse::intersect (Ray &ray, const IsecCtx &isec_ctx) const
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
Ellipse::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();

  // Center of ellipse.
  //
  Pos center = ellipse->corner + ellipse->edge1 / 2 + ellipse->edge2 / 2;

  // Calculate the normal and tangent vectors.
  //
  Vec norm = cross (ellipse->edge1, ellipse->edge2).unit ();
  Vec s = (point - center).unit ();
  Vec t = cross (norm, s);

  Intersect isec (ray, ellipse, Frame (point, s, t, norm), trace);

  isec.no_self_shadowing = true;

  return isec;
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Ellipse::shadow (const ShadowRay &ray) const
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
Ellipse::bbox () const
{
  // This could be a bit more tight...
  //
  BBox bbox (corner);
  bbox += corner + edge1;
  bbox += corner + edge2;
  bbox += corner + edge1 + edge2;
  return bbox;
}
