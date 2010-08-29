// tripar-isec.h -- Triangle/parallelogram intersection  -*- coding: utf-8 -*-
//
//  Copyright (C) 2005, 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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


// Return true if the triangle or parallelogram defined by the points
// CORNER, CORNER+EDGE1, and CORNER+EDGE2, and also CORNER+EDGE1+EDGE2
// if PARALLELOGRAM is true, is intersected by a ray from RAY_ORIGIN
// in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.  Only intersections with a parameter distance of
// MIN_T or greater are considered.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
tripar_intersects (const TPos<T> &corner,
		   const TVec<T> &edge1, const TVec<T> &edge2,
		   bool parallelogram,
		   const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		   T min_t, T &t, T &u, T &v)
{
  // Begin calculating the determinant (also used to calculate U parameter).
  //
  TVec<T> pvec = cross (ray_dir, edge2);

  // If the determinant is near zero, the ray lies in the plane of the triangle.
  //
  T det = dot (edge1, pvec);

  if (det > -Eps && det < Eps)
    return false;

  T inv_det = 1.0 / det;

  // Calculate distance from the corner to ray origin.
  //
  TVec<T> tvec = ray_origin - corner;

  // Calculate U parameter and test bounds.
  //
  u = dot (tvec, pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
    return false;

  // Prepare to test V parameter.
  //
  TVec<T> qvec = cross (tvec, edge1);

  // Calculate V parameter.
  //
  v = dot (ray_dir, qvec) * inv_det;

  // Test V parameter bounds.
  //
  // Note this is the only difference between a triangle and a
  // parallelogram -- whether U and V are independent, or whether we
  // limit their sum to 1.
  //
  if (parallelogram)
    {
      if (v < 0.0 || v > 1.0)
	return false;
    }
  else
    {
      if (v < 0.0 || u + v > 1.0)
	return false;
    }

  // The ray intersects the triangle/parallelogram; see if the intersection
  // point lies within the ray bounds.
  //
  t = dot (edge2, qvec) * inv_det;

  return t > min_t;
}

// Return true if the triangle or parallelogram defined by the points
// CORNER, CORNER+EDGE1, and CORNER+EDGE2, and also CORNER+EDGE1+EDGE2
// if PARALLELOGRAM is true, is intersected by a ray from RAY_ORIGIN
// in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
tripar_intersects (const TPos<T> &corner,
		   const TVec<T> &edge1, const TVec<T> &edge2,
		   bool parallelogram,
		   const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		   T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, parallelogram,
			    ray_origin, ray_dir, T(0), t, u, v);
}


// Return true if the triangle or parallelogram defined by the points
// CORNER, CORNER+EDGE1, and CORNER+EDGE2, and also CORNER+EDGE1+EDGE2
// if PARALLELOGRAM is true, is intersected by the ray RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY's dir field required to reach the intersection
// point from RAY's origin.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
tripar_intersects (const TPos<T> &corner,
		   const TVec<T> &edge1, const TVec<T> &edge2,
		   bool parallelogram,
		   const TRay<T> &ray,
		   T &t, T &u, T &v)
{
  return
    (tripar_intersects (corner, edge1, edge2, parallelogram,
			ray.origin, ray.dir, ray.t0, t, u, v)
     && t < ray.t1);
}



// Convenience wrappers for triangles.

// Return true if the triangle defined by the points CORNER,
// CORNER+EDGE1, and CORNER+EDGE2, is intersected by a ray from
// RAY_ORIGIN in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.  Only intersections with a parameter distance of
// MIN_T or greater are considered.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
triangle_intersects (const TPos<T> &corner,
		     const TVec<T> &edge1, const TVec<T> &edge2,
		     const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		     T min_t, T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false,
			    ray_origin, ray_dir, min_t, t, u, v);
}

// Return true if the triangle defined by the points CORNER,
// CORNER+EDGE1, and CORNER+EDGE2, is intersected by a ray from
// RAY_ORIGIN in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
triangle_intersects (const TPos<T> &corner,
		     const TVec<T> &edge1, const TVec<T> &edge2,
		     const TPos<T> &ray_origin, const TVec<T> &ray_dir,
		     T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false,
			    ray_origin, ray_dir, t, u, v);
}


// Return true if the triangle defined by the points CORNER,
// CORNER+EDGE1, and CORNER+EDGE2 is intersected by the ray RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY's dir field required to reach the intersection
// point from RAY's origin.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
triangle_intersects (const TPos<T> &corner,
		     const TVec<T> &edge1, const TVec<T> &edge2,
		     const TRay<T> &ray,
		     T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false, ray, t, u, v);
}



// Convenience wrappers for parallelograms.

// Return true if the parallelogram defined by the points CORNER,
// CORNER+EDGE1, CORNER+EDGE2, and CORNER+EDGE1+EDGE2, is intersected
// by a ray from RAY_ORIGIN in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.  Only intersections with a parameter distance of
// MIN_T or greater are considered.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
parallelogram_intersects (const TPos<T> &corner,
			  const TVec<T> &edge1, const TVec<T> &edge2,
			  const TPos<T> &ray_origin, const TVec<T> &ray_dir,
			  T min_t, T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false,
			    ray_origin, ray_dir, min_t, t, u, v);
}

// Return true if the parallelogram defined by the points CORNER,
// CORNER+EDGE1, CORNER+EDGE2, and CORNER+EDGE1+EDGE2, is intersected
// by a ray from RAY_ORIGIN in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
parallelogram_intersects (const TPos<T> &corner,
			  const TVec<T> &edge1, const TVec<T> &edge2,
			  const TPos<T> &ray_origin, const TVec<T> &ray_dir,
			  T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false,
			    ray_origin, ray_dir, t, u, v);
}


// Return true if the parallelogram defined by the points CORNER,
// CORNER+EDGE1, CORNER+EDGE2, and CORNER+EDGE1+EDGE2, is intersected
// by the ray RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY's dir field required to reach the intersection
// point from RAY's origin.
//
// The baycentric coordinates of the intersection point are returned
// in the out-parameters U and V.
//
template<typename T>
inline bool
parallelogram_intersects (const TPos<T> &corner,
			  const TVec<T> &edge1, const TVec<T> &edge2,
			  const TRay<T> &ray,
			  T &t, T &u, T &v)
{
  return tripar_intersects (corner, edge1, edge2, false, ray, t, u, v);
}


}

#endif /* __TRIPAR_ISEC_H__ */

// arch-tag: 5f366ddc-c590-4813-a245-7551784eea7c
