// tripar.cc -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"

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
  dist_t t;
  float u, v;
  if (intersects (ray, t, u, v))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, *this, u, v);
    }

  return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Tripar::IsecInfo::make_intersect (const Media &media, RenderContext &context) const
{
  Pos point = ray.end ();

  dist_t e1_len = tripar.e1.length (), inv_e1_len = 1 / e1_len;

  // Calculate the normal and tangent vectors.
  //
  Vec norm = cross (tripar.e2, tripar.e1).unit ();
  Vec s = tripar.e1 * inv_e1_len;
  Vec t = cross (s, norm);

  // Normal frame.
  //
  Frame normal_frame (point, s, t, norm);

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  Vec oe2 = normal_frame.to (tripar.e2); // tripar.e2 in object space
  dist_t duds = inv_e1_len;
  dist_t dvds = oe2.x ? 1 / oe2.x : 0;
  dist_t dvdt = oe2.y ? 1 / oe2.y : 0;
  UV dTds (duds, dvds), dTdt (0, dvdt);

  return Intersect (ray, media, context, *tripar.material,
		    normal_frame, UV (u, v), dTds, dTdt);
}

// Return the texture-coordinates of this intersection.
//
TexCoords
Tripar::IsecInfo::tex_coords () const
{
  return TexCoords (ray.end(), UV (u, v));
}

// Return the normal of this intersection (in the world frame).
//
Vec
Tripar::IsecInfo::normal () const
{
  return cross (tripar.e2, tripar.e1).unit ();
}

// Return true if this surface intersects RAY.
//
bool
Tripar::intersects (const Ray &ray, RenderContext &) const
{
  dist_t t;
  float u, v;
  return intersects (ray, t, u, v);
}

// Return true if this surface completely occludes RAY.  If it does
// not completely occlude RAY, then return false, and multiply
// TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
// MEDIUM.
//
// Note that this method does not try to handle non-trivial forms of
// transparency/translucency (for instance, a "glass" material is
// probably considered opaque because it changes light direction as
// well as transmitting it).
//
bool
Tripar::occludes (const Ray &ray, const Medium &medium,
		  Color &total_transmittance, RenderContext &)
  const
{
  dist_t t;
  float u, v;
  if (intersects (ray, t, u, v))
    {
      // avoid calculating texture coords if possible
      if (material->fully_occluding ())
	return true;

      IsecInfo isec_info (Ray (ray, t), *this, u, v);
      return material->occludes (isec_info, medium, total_transmittance);
    }
  return false;
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

// Return a sampler for this surface, or zero if the surface doesn't
// support sampling.  The caller is responsible for destroying
// returned samplers.
//
Surface::Sampler *
Tripar::make_sampler () const
{
  return new Sampler (*this);
}


// Sphere::Sampler

// Return a sample of this surface.
//
Surface::Sampler::AreaSample
Tripar::Sampler::sample (const UV &param) const
{
  float u = param.u, v = param.v;
  
  // normal
  Vec norm = cross (tripar.e2, tripar.e1);
  float area = norm.length ();
  norm /= area;			// normalize normal :)

  // If this is a triangle (rather than a parallelogram), then fold the
  // u/v parameters as necessary to stay within the triangle.
  //
  if (!tripar.parallelogram && u + v > 1)
    {
      u = 1 - u;
      v = 1 - v;
      area *= 0.5f;
    }

  // position
  Pos pos = tripar.v0 + tripar.e1 * u + tripar.e2 * v;

  // pdf
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
Tripar::Sampler::eval_from_viewpoint (const Pos &viewpoint, const Vec &dir)
  const
{
  dist_t t;
  UV param;
  if (tripar.intersects (viewpoint, dir, t, param.u, param.v))
    return sample_from_viewpoint (viewpoint, param);
  return AngularSample ();
}


// arch-tag: 962df04e-4c0a-4754-ac1a-f506d4e77c4e
