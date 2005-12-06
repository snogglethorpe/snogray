// triangle.cc -- Triangle surface		-*- coding: utf-8 -*-
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "triangle.h"

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
Triangle::intersection_distance (const Ray &ray, IsecParams &isec_params,
				 unsigned num)
  const
{
  if (num != 0)
    return 0;

  /*
    This algorithm from:

       Fast, Minimum Storage Ray-Triangle Intersection

       Tomas Möller
       Prosolvia Clarus AB
       Sweden
       tompa@clarus.se

       Ben Trumbore
       Cornell University
       Ithaca, New York
       wbt@graphics.cornell.edu
  */

  /* find vectors for two edges sharing vert0 */
  Vec edge1 = v1 - v0;
  Vec edge2 = v2 - v0;

  /* begin calculating determinant - also used to calculate U parameter */
  Vec pvec = ray.dir.cross (edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  double det = edge1.dot (pvec);

  if (det > -Eps && det < Eps)
    return 0;

  double inv_det = 1.0 / det;

  /* calculate distance from vert0 to ray origin */
  Vec tvec = ray.origin - v0;

  /* calculate U parameter and test bounds */
  double u = tvec.dot (pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
    return 0;

  /* prepare to test V parameter */
  Vec qvec = tvec.cross (edge1);

  /* calculate V parameter and test bounds */
  double v = ray.dir.dot (qvec) * inv_det;
  if (v < 0.0 || u + v > 1.0)
    return 0;

  /* calculate t, ray intersects triangle */
  dist_t t = edge2.dot (qvec) * inv_det;

  isec_params.u = u;
  isec_params.v = v;

  return t;
}

Intersect
Triangle::intersect_info (const Ray &ray, const IsecParams &isec_params)
  const
{
  return Intersect (ray, this, ray.end (), ((v1 - v0).cross (v1 - v2)).unit ());
}

// Return a bounding box for this surface.
BBox
Triangle::bbox () const
{
  BBox bbox (v0);
  bbox.include (v1);
  bbox.include (v2);
  return bbox;
}

// arch-tag: 962df04e-4c0a-4754-ac1a-f506d4e77c4e
