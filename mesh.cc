// mesh.cc -- Mesh object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mesh.h"

#include "intersect.h"

using namespace Snogray;



class Mesh::Part
{
  typedef unsigned char point_index_t;
  typedef unsigned char vertex_index_t;

  // A vector of points, each referred to by a point_index_t index 
  Pos *points;

  // A list of vertexes, each represented as three contiguous indices
  // into the `points' vector.
  point_index_t *vertices;

  // A vector of Mesh::Triangle objects that use this part.
  Triangle *triangles;

  // Next part in a linked list of them
  Part *next;
};

class Mesh::Triangle : Obj
{
public:
  const Pos 
  const Part::point_index_t *v2p = part->vertices + v2;
  const Pos &v2x = part->points[v2p[0]];
  const Pos &v2y = part->points[v2p[1]];
  const Pos &v2z = part->points[v2p[2]];

  Part *part;
  Part::vertex_index_t v0i, v1i, v2i;
};



dist_t
Mesh::Triangle::intersection_distance (const Ray &ray) const
{
  const Part::point_index_t *v0p = part->vertices + v0;
  const Pos &v0x = part->points[v0p[0]];
  const Pos &v0y = part->points[v0p[1]];
  const Pos &v0z = part->points[v0p[2]];

  const Part::point_index_t *v1p = part->vertices + v1;
  const Pos &v1x = part->points[v1p[0]];
  const Pos &v1y = part->points[v1p[1]];
  const Pos &v1z = part->points[v1p[2]];

  const Part::point_index_t *v2p = part->vertices + v2;
  const Pos &v2x = part->points[v2p[0]];
  const Pos &v2y = part->points[v2p[1]];
  const Pos &v2z = part->points[v2p[2]];

  float a = v0x - v1x; 
  float b = v0y - v1y; 
  float c = v0z - v1z; 
  float d = v0x - v2x; 
  float e = v0y - v2y; 
  float f = v0z - v2z; 
  float g = ray.dir.x;
  float h = ray.dir.y; 
  float i = ray.dir.z; 
  float j = v0x - ray.origin.x; 
  float k = v0y - ray.origin.y; 
  float l = v0z - ray.origin.z; 
	
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
Mesh::normal (const Pos &point, const Vec &eye_dir) const
{
  Vec norm = ((v1 - v0).cross (v1 - v2)).unit ();

  // Meshs are visible from both sides, so keep the normal sane
  if (norm.dot (eye_dir) < 0)
    norm = -norm;

  return norm;
}

// Return a bounding box for this object.
BBox
Mesh::bbox () const
{
  BBox bbox (v0);
  bbox.include (v1);
  bbox.include (v2);
  return bbox;
}

// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
