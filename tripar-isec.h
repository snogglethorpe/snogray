// tripar-isec.h -- Triangle/parallelogram intersection  -*- coding: utf-8 -*-
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRIPAR_ISEC_H__
#define __TRIPAR_ISEC_H__

#include "pos.h"
#include "vec.h"
#include "ray.h"


//
// The triangle/parallelogram intersection algorithm used in this file is
// from:
//
//   "Fast, Minimum Storage Ray-Triangle Intersection"
//
//    Tomas Möller
//    Prosolvia Clarus AB
//    Sweden
//    tompa@clarus.se
//
//    Ben Trumbore
//    Cornell University
//    Ithaca, New York
//    wbt@graphics.cornell.edu
//


namespace snogray {


// Return true if RAY intersects the triangle or parallelogram defined by
// the points CORNER, CORNER+EDGE1, and CORNER+EDGE2, and if PARALLELOGRAM
// is true, also CORNER+EDGE1+EDGE2.
//
template<typename T>
inline bool
tripar_intersect (const TPos<T> &corner,
		  const TVec<T> &edge1, const TVec<T> &edge2,
		  bool parallelogram,
		  const TRay<T> &ray)
{
  // Begin calculating the determinant (also used to calculate U parameter).
  //
  TVec<T> pvec = cross (ray.dir, edge2);

  // If the determinant is near zero, the ray lies in the plane of the triangle.
  //
  T det = dot (edge1, pvec);

  if (det > -Eps && det < Eps)
    return false;

  T inv_det = 1.0 / det;

  // Calculate distance from the corner to ray origin.
  //
  TVec<T> tvec = ray.origin - corner;

  // Calculate U parameter and test bounds.
  //
  T _u = dot (tvec, pvec) * inv_det;
  if (_u < 0.0 || _u > 1.0)
    return false;

  // Prepare to test V parameter.
  //
  TVec<T> qvec = cross (tvec, edge1);

  // Calculate V parameter.
  //
  T _v = dot (ray.dir, qvec) * inv_det;

  // Test V parameter bounds.
  //
  // Note this is the only difference between a triangle and a
  // parallelogram -- whether U and V are independent, or whether we
  // limit their sum to 1.
  //
  if (parallelogram)
    {
      if (_v < 0.0 || _v > 1.0)
	return false;
    }
  else
    {
      if (_v < 0.0 || _u + _v > 1.0)
	return false;
    }

  // The ray intersects the triangle/parallelogram; see if the intersection
  // point lies within the ray bounds.
  //
  T t = dot (edge2, qvec) * inv_det;
  return (t > ray.t0 && t < ray.t1);
}

// If RAY intersects the triangle or parallelogram defined by the points
// CORNER, CORNER+EDGE1, and CORNER+EDGE2, and if PARALLELOGRAM is true,
// also CORNER+EDGE1+EDGE2, then update RAY's t1 field to be the distance
// to the intersection, set U and V to the parametric values of the
// intersection point, and return true.  If the ray doesn't intersect,
// return false.
//
template<typename T>
inline bool
tripar_intersect (const TPos<T> &corner,
		  const TVec<T> &edge1, const TVec<T> &edge2,
		  bool parallelogram,
		  TRay<T> &ray, T &u, T &v)
{
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

  // Begin calculating the determinant (also used to calculate U parameter).
  //
  TVec<T> pvec = cross (ray.dir, edge2);

  // If the determinant is near zero, the ray lies in the plane of the triangle.
  //
  T det = dot (edge1, pvec);

  if (det > -Eps && det < Eps)
    return false;

  T inv_det = 1.0 / det;

  // Calculate distance from the corner to ray origin.
  //
  TVec<T> tvec = ray.origin - corner;

  // Calculate U parameter and test bounds.
  //
  T _u = dot (tvec, pvec) * inv_det;
  if (_u < 0.0 || _u > 1.0)
    return false;

  // Prepare to test V parameter.
  //
  TVec<T> qvec = cross (tvec, edge1);

  // Calculate V parameter.
  //
  T _v = dot (ray.dir, qvec) * inv_det;

  // Test V parameter bounds.
  //
  // Note this is the only difference between a triangle and a
  // parallelogram -- whether U and V are independent, or whether we
  // limit their sum to 1.
  //
  if (parallelogram)
    {
      if (_v < 0.0 || _v > 1.0)
	return false;
    }
  else
    {
      if (_v < 0.0 || _u + _v > 1.0)
	return false;
    }

  // The ray intersects the triangle/parallelogram; see if the intersection
  // point lies within the ray bounds.
  //
  T t = dot (edge2, qvec) * inv_det;
  if (t > ray.t0 && t < ray.t1)
    {
      ray.t1 = t;
      u = _u;
      v = _v;
      return true;
    }

  // Nope...
  //
  return false;
}

// If RAY intersects the triangle defined by the points CORNER,
// CORNER+EDGE1, and CORNER+EDGE2, then update RAY's t1 field to be the
// distance to the intersection, set U and V to the parametric values of
// the intersection point, and return true.  If the ray doesn't intersect,
// return false.
//
template<typename T>
inline bool
triangle_intersect (const TPos<T> &corner,
		    const TVec<T> &edge1, const TVec<T> &edge2,
		    TRay<T> &ray, T &u, T &v)
{
  return tripar_intersect (corner, edge1, edge2, false, ray, u, v);
}

// If RAY intersects the parallelogram defined by the points CORNER,
// CORNER+EDGE1, CORNER+EDGE2, and CORNER+EDGE1+EDGE2, then update RAY's t1
// field to be the distance to the intersection, set U and V to the
// parametric values of the intersection point, and return true.  If the
// ray doesn't intersect, return false.
//
template<typename T>
inline bool
parallelogram_intersect (const TPos<T> &corner,
			 const TVec<T> &edge1, const TVec<T> &edge2,
			 TRay<T> &ray, T &u, T &v)
{
  return tripar_intersect (corner, edge1, edge2, true, ray, u, v);
}

// Return true if RAY intersects the triangle defined by the points CORNER,
// CORNER+EDGE1, and CORNER+EDGE2.
//
template<typename T>
inline bool
triangle_intersect (const TPos<T> &corner,
		    const TVec<T> &edge1, const TVec<T> &edge2,
		    const TRay<T> &ray)
{
  return tripar_intersect (corner, edge1, edge2, false, ray);
}


}

#endif /* __TRIPAR_ISEC_H__ */

// arch-tag: 5f366ddc-c590-4813-a245-7551784eea7c
