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
Ellipse::IsecInfo::make_intersect (const Media &media, RenderContext &context)
  const
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

  Intersect isec (ray, media, context, *ellipse->material,
		  norm_frame, tex_coords, dTds, dTdt);

  isec.no_self_shadowing = ellipse;

  return isec;
}

// Return true if this surface intersects RAY.
//
bool
Ellipse::intersects (const Ray &ray, RenderContext &) const
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

// Return a sampler for this surface, or zero if the surface doesn't
// support sampling.  The caller is responsible for destroying
// returned samplers.
//
Surface::Sampler *
Ellipse::make_sampler () const
{
  return new Sampler (*this);
}


// Ellipse::Sampler

// Return a sample of this surface.
//
Surface::Sampler::AreaSample
Ellipse::Sampler::sample (const UV &param) const
{
  // position
  Pos pos = ellipse.corner + ellipse.edge1 * param.u + ellipse.edge2 * param.v;

  // normal
  Vec norm = cross (ellipse.edge2, ellipse.edge1);
  dist_t norm_unnorm_len = norm.length ();
  norm /= norm_unnorm_len;	// normalize normal :)

  // pdf
  dist_t area = norm_unnorm_len * PIf * 0.25f;
  float pdf = 1 / area;

  return AreaSample (pos, norm, pdf);
}

// If a ray from VIEWPOINT in direction DIR intersects this
// surface, return an AngularSample as if the
// Surface::Sampler::sample_from_viewpoint method had returned a
// sample at the intersection position.  Otherwise, return an
// AngularSample with a PDF of zero.
//
Surface::Sampler::AngularSample
Ellipse::Sampler::eval_from_viewpoint (const Pos &viewpoint, const Vec &dir)
  const
{
  dist_t t;
  UV param;
  if (ellipse.intersects (viewpoint, dir, t, param.u, param.v))
    return sample_from_viewpoint (viewpoint, param);
  return AngularSample ();
}
