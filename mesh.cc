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



class Mesh::Triangle : Obj
{
public:

  const Pos &v0 () { return mesh->vertices[v0i]; }
  const Pos &v1 () { return mesh->vertices[v1i]; }
  const Pos &v2 () { return mesh->vertices[v2i]; }

  Mesh *mesh;

  vertex_index_t v0i, v1i, v2i;
};



Mesh *
Mesh::read_msh_file (istream stream)
{
  Mesh *mesh = new Mesh ();

  stream >> mesh->num_vertices;
  stream >> mesh->num_triangles;

  // For the time being, we only support meshes up to 65536 vertices
  if (num_vertices > 65536)
    throw TOO_MANY_VERTICES_ERROR;

  mesh->vertices = new Pos[mesh->num_vertices];
  mesh->triangles = new Triangle[mesh->num_triangles];

  char kw[10];

  stream >> kw;
  if (strcmp (kw, "vertices") != 0)
    throw FILE_FORMAT_ERROR;

  for (unsigned i = 0; i < mesh->num_vertices; i++)
    {
      stream >> vertices[i].x;
      stream >> vertices[i].y;
      stream >> vertices[i].z;
    }

  stream >> kw;
  if (strcmp (kw, "triangles") != 0)
    throw FILE_FORMAT_ERROR;

  for (unsigned i = 0; i < meshnum_triangles; i++)
    {
      stream >> triangles[i].v0i;
      stream >> triangles[i].v1i;
      stream >> triangles[i].v2i;
    }

  
}



dist_t
Mesh::Triangle::intersection_distance (const Ray &ray) const
{
  const Pos &_v0 = v0 (), &_v1 = v1 (), &_v2 = v2 ();

  float a = _v0.x - _v1.x; 
  float b = _v0.y - _v1.y; 
  float c = _v0.z - _v1.z; 
  float d = _v0.x - _v2.x; 
  float e = _v0.y - _v2.y; 
  float f = _v0.z - _v2.z; 
  float g = ray.dir.x;
  float h = ray.dir.y; 
  float i = ray.dir.z; 
  float j = _v0.x - ray.origin.x; 
  float k = _v0.y - ray.origin.y; 
  float l = _v0.z - ray.origin.z; 
	
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
  return ((v1() - v0()).cross (v1() - v2())).unit ();
}

// Return a bounding box for this object.
BBox
Mesh::bbox () const
{
  BBox bbox (v0 ());
  bbox.include (v1 ());
  bbox.include (v2 ());
  return bbox;
}

// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
