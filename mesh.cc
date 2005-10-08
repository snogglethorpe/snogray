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

#include <fstream>

#include "excepts.h"
#include "mesh.h"

using namespace Snogray;
using namespace std;



class Mesh::Triangle : public Obj
{
public:

  virtual dist_t intersection_distance (const Ray &ray) const;

  // Returns the normal vector for this surface at POINT.
  // EYE_DIR points to the direction the objects is being viewed from;
  // this can be used by dual-sided objects to decide which side's
  // normal to return.
  //
  virtual Vec normal (const Pos &point, const Vec &eye_dir) const;

  // Return a bounding box for this object.
  //
  virtual BBox bbox () const;

  const Pos &v0 () const { return mesh->vertices[v0i]; }
  const Pos &v1 () const { return mesh->vertices[v1i]; }
  const Pos &v2 () const { return mesh->vertices[v2i]; }

  Mesh *mesh;

  vertex_index_t v0i, v1i, v2i;
};


// Generic mesh-file loading

void
Mesh::load (const char *file_name)
{
  ifstream stream (file_name);

  if (stream)
    try
      { 
	const char *file_ext = rindex (file_name, '.');

	if (! file_ext)
	  throw
	    file_error ("no filename extension to determine mesh file format");
	else
	  file_ext++;

	if (strcmp (file_ext, "msh") == 0)
	  load_msh_file (stream);
      }
    catch (std::runtime_error &err)
      {
	throw file_error (string (file_name)
			  + ": error reading mesh file: "
			  + err.what ());
      }
  else
    throw file_error (string (file_name) + ": cannot open mesh file");
}


// .msh mesh-file format

void
Mesh::load_msh_file (istream &stream)
{
  if (triangles || vertices)
    throw std::runtime_error ("mesh already initialized");

  stream >> num_vertices;
  stream >> num_triangles;

  // For the time being, we only support meshes up to 65536 vertices
  if (num_vertices > 65536)
    throw bad_format ("too many vertices (must be less than 65536)");

  vertices = new Pos[num_vertices];
  triangles = new Triangle[num_triangles];

  char kw[10];

  stream >> kw;
  if (strcmp (kw, "vertices") != 0)
    throw bad_format ();

  for (unsigned i = 0; i < num_vertices; i++)
    {
      stream >> vertices[i].x;
      stream >> vertices[i].y;
      stream >> vertices[i].z;
    }

  stream >> kw;
  if (strcmp (kw, "triangles") != 0)
    throw bad_format ();

  for (unsigned i = 0; i < num_triangles; i++)
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
Mesh::Triangle::normal (const Pos &point, const Vec &eye_dir) const
{
  return ((v1() - v0()).cross (v1() - v2())).unit ();
}

// Return a bounding box for this object.
BBox
Mesh::Triangle::bbox () const
{
  BBox bbox (v0 ());
  bbox.include (v1 ());
  bbox.include (v2 ());
  return bbox;
}

// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
