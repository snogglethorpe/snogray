// ellipse.cc -- Ellipse surface
//
//  Copyright (C) 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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
// placement-new with CONTEXT); otherwise return zero.
//
const Surface::IsecInfo *
Ellipse::intersect (Ray &ray, RenderContext &context) const
{
  dist_t t, u, v;
  if (intersects (ray, t, u, v))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, this);
    }
  return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Ellipse::IsecInfo::make_intersect (Trace &trace, RenderContext &context) const
{
  Pos point = ray.end ();

  // The ellipse's two "radii".
  //
  Vec rad1 = ellipse->edge1 / 2;
  Vec rad2 = ellipse->edge2 / 2;
  dist_t inv_rad1_len = 1 / rad1.length ();
  dist_t inv_rad2_len = 1 / rad1.length ();

  // Center of ellipse.
  //
  Pos center = ellipse->corner + rad1 + rad2;

  // Calculate the normal and tangent vectors.
  //
  Vec norm = cross (rad2, rad1).unit ();
  Vec s = rad1 * inv_rad1_len;
  Vec t = cross (s, norm);

  // Normal frame.
  //
  Frame norm_frame (point, s, t, norm);

  // 2d texture coordinates.
  //
  Vec ocent = norm_frame.to (center);
  UV tex_coords (-ocent.x * inv_rad1_len * 0.5f + 0.5f,
		 -ocent.y * inv_rad2_len * 0.5f + 0.5f);
  //
  // TEX_COORDS will not be "correct" in case where edge1 and edge2 are
  // skewed (not perpendicular); it's not really hard to calculate it
  // correctly in that case, but a bit annoying.

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  UV dTds (0.5f * inv_rad1_len, 0), dTdt (0, 0.5f * inv_rad2_len);

  Intersect isec (ray, trace, ellipse, norm_frame, tex_coords, dTds, dTdt);

  isec.no_self_shadowing = true;

  return isec;
}

// Return true if this surface intersects RAY.
//
bool
Ellipse::intersects (const ShadowRay &ray, RenderContext &) const
{
  dist_t t, u, v;
  return intersects (ray, t, u, v);
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
