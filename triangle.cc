// triangle.cc -- Triangle surface
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
// NUM is which intersection to return, for non-flat surfaces that may
// have multiple intersections -- 0 for the first, 1 for the 2nd, etc
// (flat surfaces will return failure for anything except 0).
//
dist_t
Triangle::intersection_distance (const Ray &ray, unsigned num) const
{
  if (num != 0)
    return 0;

  double a = v0.x - v1.x; 
  double b = v0.y - v1.y; 
  double c = v0.z - v1.z; 
  double d = v0.x - v2.x; 
  double e = v0.y - v2.y; 
  double f = v0.z - v2.z; 
  double g = ray.dir.x;
  double h = ray.dir.y; 
  double i = ray.dir.z; 
  double j = v0.x - ray.origin.x; 
  double k = v0.y - ray.origin.y; 
  double l = v0.z - ray.origin.z; 
	
  double one = a*k - j*b; 
  double two = j*c - a*l; 
  double three = b*l - k*c; 

  double four = (e*i - h*f); 
  double five = (g*f - d*i); 
  double six = (d*h - e*g); 

  double M = a*four + b*five + c*six;
	
  //	 compute t 
  double t = -(f*one + e*two + d*three) / M; 
  if (t < -Eps)
    return 0;
		
  //	 compute R
  double R = (i*one + h*two + g*three) / M; 
  if (R < -Eps || R > 1 + Eps)
    return 0;
	
  // 	compute B
  double B = (j*four + k*five + l*six) / M; 
  if(B < -Eps || B > (1 - R - Eps))
    return 0;

  return t;
}

Vec
Triangle::normal (const Pos &point, const Vec &incoming) const
{
  return ((v1 - v0).cross (v1 - v2)).unit ();
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
