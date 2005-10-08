// triangle.cc -- Triangle object
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

dist_t
Triangle::intersection_distance (const Ray &ray) const
{
  float a = v0.x - v1.x; 
  float b = v0.y - v1.y; 
  float c = v0.z - v1.z; 
  float d = v0.x - v2.x; 
  float e = v0.y - v2.y; 
  float f = v0.z - v2.z; 
  float g = ray.dir.x;
  float h = ray.dir.y; 
  float i = ray.dir.z; 
  float j = v0.x - ray.origin.x; 
  float k = v0.y - ray.origin.y; 
  float l = v0.z - ray.origin.z; 
	
  float one = a*k - j*b; 
  float two = j*c - a*l; 
  float three = b*l - k*c; 

  float four = (e*i - h*f); 
  float five = (g*f - d*i); 
  float six = (d*h - e*g); 

  float M = a*four + b*five + c*six;
	
  //	 compute t 
  float t = -(f*one + e*two + d*three) / M; 
  if (t < 0)
    return 0;
		
  //	 compute R
  float R = (i*one + h*two + g*three) / M; 
  if (R < 0 || R > 1)
    return 0;
	
  // 	compute B
  float B = (j*four + k*five + l*six) / M; 
  if(B < 0 || B > (1 - R))
    return 0;

  return t;
}

Vec
Triangle::normal (const Pos &point, const Vec &eye_dir) const
{
  Vec norm = ((v1 - v0).cross (v1 - v2)).unit ();

  // Triangles are visible from both sides, so keep the normal sane
  if (norm.dot (eye_dir) < 0)
    norm = -norm;

  return norm;
}

// Return a bounding box for this object.
BBox
Triangle::bbox () const
{
  BBox bbox (v0);
  bbox.include (v1);
  bbox.include (v2);
  return bbox;
}

// arch-tag: 962df04e-4c0a-4754-ac1a-f506d4e77c4e
