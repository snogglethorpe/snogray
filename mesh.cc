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

  // Returns the material this object is made from
  //
  virtual const Material *material () const;

  // Vertex NUM of this triangle
  //
  const Pos &v (unsigned num) const { return mesh->vertices[vi[num]]; }

  // Normal of vertex NUM (assuming this mesh contains vertex normals!)
  //
  const Vec &vnorm (unsigned num) const { return mesh->vertex_normals[vi[num]];}

  const Vec raw_normal () const
  {
    return ((v(1) - v(0)).cross (v(2) - v(1))).unit ();
  }

  Mesh *mesh;

  // Indices into mesh vertices array
  //
  unsigned vi[3];
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
      for (unsigned num = 0; num < 3; num++)
	stream >> triangles[i].vi[num];

      triangles[i].mesh = this;
    }
}



dist_t
Mesh::Triangle::intersection_distance (const Ray &ray) const
{
  const Pos &_v0 = v (0), &_v1 = v (1), &_v2 = v (2);

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
  if (mesh->vertex_normals)
    {
      Vec norm;

#if 1
      /* score: 2 (no-square), 5 (square)*/
      dist_t vdists[3];

      for (unsigned num = 0; num < 3; num++)
	{
	  vdists[num] = powf ((v (num) - point).length (), 3);
	//	vdists[num] *= vdists[num];
	}

      norm += vnorm (0) * vdists[1] * vdists[2];
      norm += vnorm (1) * vdists[0] * vdists[2];
      norm += vnorm (2) * vdists[0] * vdists[1];
#elif 1
      /* score: 3 */
      dist_t edge01 = v(0).dist (v(1));
      dist_t edge02 = v(0).dist (v(2));
      dist_t edge12 = v(1).dist (v(2));

      norm += vnorm(0) * ((edge01 > edge02 ? edge01 : edge02) - v(0).dist (point));
      norm += vnorm(1) * ((edge01 > edge12 ? edge01 : edge12) - v(1).dist (point));
      norm += vnorm(2) * ((edge02 > edge12 ? edge02 : edge12) - v(2).dist (point));
#else
      dist_t vdists[3];

      for (unsigned num = 0; num < 3; num++)
	vdists[num] = (v (num) - point).length ();

      norm += vnorm (0) / (vvdists[0];
      norm += vnorm (1) / vdists[1];
      norm += vnorm (2) / vdists[2];
#endif

      norm = norm.unit ();

      // Triangles are visible from both sides, so keep the normal sane
      if (norm.dot (eye_dir) < 0)
	norm = -norm;

      return norm;
    }
  else
    {
      Vec norm (raw_normal ());

      // Triangles are visible from both sides, so keep the normal sane
      if (norm.dot (eye_dir) < 0)
	norm = -norm;

      return norm;
    }
}

// Return a bounding box for this object.
BBox
Mesh::Triangle::bbox () const
{
  BBox bbox (v (0));
  bbox.include (v (1));
  bbox.include (v (2));
  return bbox;
}

// Returns the material this object is made from
//
const Material *
Mesh::Triangle::material () const
{
  return mesh->material ();
}



Mesh::~Mesh ()
{
  if (triangles)
    delete[] triangles;
  if (vertices)
    delete[] vertices;
}

void
Mesh::compute_vertex_normals ()
{
  if (! vertex_normals)
    {
      vertex_normals = new Vec[num_vertices];

      unsigned *face_counts = new unsigned[num_vertices];

      for (unsigned v = 0; v < num_vertices; v++)
	face_counts[v] = 0;

      for (unsigned t = 0; t < num_triangles; t++)
	{
	  const Triangle &triang = triangles[t];
	  const Vec norm (triang.raw_normal ());

	  for (unsigned num = 0; num < 3; num++)
	    {
	      unsigned v = triang.vi[num];
	      vertex_normals[v] += norm;
	      face_counts[v] ++;
	    }
	}
	
      for (unsigned v = 0; v < num_vertices; v++)
	if (face_counts[v] > 1)
	  vertex_normals[v] /= face_counts[v];

      delete[] face_counts;
    }
}

// Add this (or some other ...) objects to SPACE
//
void
Mesh::add_to_space (Voxtree &space)
{
  if (! triangles)
    throw std::runtime_error ("cannot instantiate unloaded mesh");

  compute_vertex_normals ();

  for (unsigned i = 0; i < num_triangles; i++)
    triangles[i].add_to_space (space);
}

// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
