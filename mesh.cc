// mesh.cc -- Mesh surface
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
#include <fstream>

#include "excepts.h"
#include "mesh.h"
#include "tessel.h"
#include "string-funs.h"

using namespace Snogray;
using namespace std;


// Add a vertex to the mesh

// This simple version always adds a new vertex
//
unsigned
Mesh::add_vertex (const Pos &pos)
{
  unsigned vert_index = vertices.size ();
  vertices.push_back (pos);
  return vert_index;
}

// This version uses VGROUP to keep track of vertex positions, and only
// adds new vertices.
//
unsigned
Mesh::add_vertex (const Pos &pos, VertexGroup &vgroup)
{
  VertexGroup::iterator prev_entry = vgroup.find (pos);

  if (prev_entry != vgroup.end ())
    return prev_entry->second;
  else
    {
      unsigned vert_index = add_vertex (pos);
      vgroup.insert (prev_entry, VertexGroup::value_type (pos, vert_index));
      return vert_index;
    }
}


// Add a vertex with normal to the mesh

// This simple version always adds a new vertex+normal
//
unsigned
Mesh::add_vertex (const Pos &pos, const Vec &normal)
{
  unsigned vert_index = vertices.size ();

  vertices.push_back (pos);

  if (vertex_normals.size() < vert_index + 1)
    vertex_normals.resize (vert_index + 1);

  vertex_normals[vert_index] = normal;

  return vert_index;
}

// This version uses VGROUP to keep track of vertex positions and normal
// values, and only adds new vertices (but a vertex with a different
// normal is considered "new").
//
unsigned
Mesh::add_vertex (const Pos &pos, const Vec &normal, VertexNormalGroup &vgroup)
{
  VertexNormalGroup::key_type key (pos, normal);
  VertexNormalGroup::iterator prev_entry = vgroup.find (key);

  if (prev_entry != vgroup.end ())
    return prev_entry->second;
  else
    {
      unsigned vert_index = add_vertex (pos, normal);
      vgroup.insert (prev_entry,
		     VertexNormalGroup::value_type (key, vert_index));
      return vert_index;
    }
}


// Add a triangle to the mesh

void
Mesh::add_triangle (unsigned v0i, unsigned v1i, unsigned v2i)
{
  Triangle triang (*this, v0i, v1i, v2i);

  triangles.push_back (triang);
}

void
Mesh::add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		    VertexGroup &vgroup)
{
  unsigned v0i = add_vertex (v0, vgroup);
  unsigned v1i = add_vertex (v1, vgroup);
  unsigned v2i = add_vertex (v2, vgroup);

  add_triangle (v0i, v1i, v2i);
}

void
Mesh::add_triangle (const Pos &v0, const Pos &v1, const Pos &v2)
{
  add_triangle (add_vertex (v0), add_vertex (v1), add_vertex (v2));
}


// Tessellation support

// Add the results of tessellating TESSEL_FUN with MAX_ERR.
//
void
Mesh::add (const Tessel::Function &tessel_fun,
	   const Tessel::MaxErrCalc &max_err)
{
  // Do the tessellation
  //
  Tessel tessel (tessel_fun, max_err);

  unsigned base_vert = vertices.size ();

  extern bool tessel_smooth;
  bool has_normals = tessel_fun.has_vertex_normals ();
  unsigned tessel_num_verts = tessel.num_vertices ();

  if (! tessel_smooth)
    has_normals = false;

  // Try to increase efficiency by pre-allocating space
  //
  triangles.reserve (triangles.size() + tessel.num_triangles ());
  vertices.reserve (base_vert + tessel_num_verts);
  if (has_normals)
    vertex_normals.reserve (base_vert + tessel_num_verts);

  // Add vertices
  //
  for (LinkedList<Tessel::Vertex>::iterator vi = tessel.vertices_begin ();
       vi != tessel.vertices_end(); vi++)
    if (has_normals)
      add_vertex (vi->pos, tessel_fun.vertex_normal (*vi));
    else
      add_vertex (vi->pos);

  // Add triangles
  //
  for (Tessel::TriangleIter ti = tessel.triangles_begin();
       ti != tessel.triangles_end(); ti++)
    add_triangle (base_vert + ti.vert1().index,
		  base_vert + ti.vert2().index,
		  base_vert + ti.vert3().index);

  if (tessel_smooth && !has_normals)
    compute_vertex_normals ();
}


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
	    file_error ("No filename extension to determine mesh file format");
	else
	  file_ext++;

	if (strcmp (file_ext, "msh") == 0)
	  load_msh_file (stream);
      }
    catch (std::runtime_error &err)
      {
	throw file_error (string (file_name)
			  + ": Error reading mesh file: "
			  + err.what ());
      }
  else
    throw file_error (string (file_name) + ": Cannot open mesh file");
}


// .msh mesh-file format

void
Mesh::load_msh_file (istream &stream)
{
  unsigned num_vertices, num_triangles;
  stream >> num_vertices;
  stream >> num_triangles;

  vertices.reserve (num_vertices);
  triangles.reserve (num_triangles);

  char kw[10];

  stream >> kw;
  if (strcmp (kw, "vertices") != 0)
    throw bad_format ();

  unsigned base_vert = vertices.size ();

  for (unsigned i = 0; i < num_vertices; i++)
    {
      coord_t x, y, z;

      stream >> x;
      stream >> y;
      stream >> z;

      add_vertex (Pos (x, y, z));
    }

  stream >> kw;
  if (strcmp (kw, "triangles") != 0)
    throw bad_format ();

  for (unsigned i = 0; i < num_triangles; i++)
    {
      unsigned v0i, v1i, v2i;

      stream >> v0i;
      stream >> v1i;
      stream >> v2i;

      add_triangle (base_vert + v0i, base_vert + v1i, base_vert + v2i);
    }
}



//
// Computing Barycentric coordinates (u, v, w) of the point p on the
// triangle with vertices v0, v1, v2:
//
//     λu = (B * (F + I) - C * (E + H)) / (A * (E + H) - B * (D + G))
//
// and
//
//     λv = (A * (F + I) - C * (D + G)) / (B * (D + G) - A * (E + H))
//
// and
//
//     λw = 1 - u - v
//
// where
//
//     A = v0.x - v2.x
//     B = v1.x - v2.x 
//     C = v2.x - p.x
//     D = v0.y - v2.y 
//     E = v1.y - v2.y 
//     F = v2.y - p.y
//     G = v0.z - v2.z 
//     H = v1.z - v2.z 
//     I = v2.z - p.z
//
// Note that:
//
//   p = v0 -> (u,v,w) = (1,0,0)
//   p = v1 -> (u,v,w) = (0,1,0)
//   p = v2 -> (u,v,w) = (0,0,1)
//
static void
compute_barycentric_coords (const Pos &p,
			    const Pos &v0, const Pos &v1, const Pos &v2,
			    double &u, double &v)
{
  double a = v0.x - v2.x;
  double b = v1.x - v2.x;
  double c = v2.x - p.x;
  double d = v0.y - v2.y;
  double e = v1.y - v2.y;
  double f = v2.y - p.y;
  double g = v0.z - v2.z;
  double h = v1.z - v2.z;
  double i = v2.z - p.z;

  u = (b * (f + i) - c * (e + h)) / (a * (e + h) - b * (d + g));
  v = (a * (f + i) - c * (d + g)) / (b * (d + g) - a * (e + h));
}

// Same, but compute w too.
//
static void
compute_barycentric_coords (const Pos &p,
			    const Pos &v0, const Pos &v1, const Pos &v2,
			    double &u, double &v, double &w)
{
  compute_barycentric_coords (p, v0, v1, v2, u, v);
  w = 1 - u - v;
}



// Return the distance from RAY's origin to the closest intersection
// of this surface with RAY, or 0 if there is none.  RAY is considered
// to be unbounded.
//
// NUM is which intersection to return, for non-flat surfaces that may
// have multiple intersections -- 0 for the first, 1 for the 2nd, etc
// (flat surfaces will return failure for anything except 0).
//
dist_t
Mesh::Triangle::intersection_distance (const Ray &ray, unsigned num) const
{
  if (num != 0)
    return 0;

  const Pos &_v0 = v (0), &_v1 = v (1), &_v2 = v (2);

  double a = _v0.x - _v1.x; 
  double b = _v0.y - _v1.y; 
  double c = _v0.z - _v1.z; 
  double d = _v0.x - _v2.x; 
  double e = _v0.y - _v2.y; 
  double f = _v0.z - _v2.z; 
  double g = ray.dir.x;
  double h = ray.dir.y; 
  double i = ray.dir.z; 
  double j = _v0.x - ray.origin.x; 
  double k = _v0.y - ray.origin.y; 
  double l = _v0.z - ray.origin.z; 
	
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
  if (R < Eps || R > 1 + Eps)
    return 0;
	
  // 	compute B
  double B = (j*four + k*five + l*six) / M; 
  if(B < Eps || B > (1 - R - Eps))
    return 0;

  return t;
}

Vec
Mesh::Triangle::normal (const Pos &point, const Vec &incoming) const
{
  Vec norm;

  if (! mesh.vertex_normals.empty ())
    {
      double w0, w1, w2;
      compute_barycentric_coords (point, v(0), v(1), v(2), w0, w1, w2);

      norm = vnorm(0) * w0;
      norm += vnorm(1) * w1;
      norm += vnorm(2) * w2;

      norm = norm.unit ();
    }
  else
    norm = raw_normal ();

  return norm;
}

// Return a bounding box for this surface.
BBox
Mesh::Triangle::bbox () const
{
  BBox bbox (v (0));
  bbox.include (v (1));
  bbox.include (v (2));
  return bbox;
}

// Returns the material this surface is made from
//
const Material *
Mesh::Triangle::material () const
{
  return mesh.material ();
}



void
Mesh::compute_vertex_normals ()
{
  unsigned num_verts = vertices.size ();
  unsigned num_triangs = triangles.size ();

  if (vertex_normals.size() < num_verts)
    {
      vector<unsigned> face_counts (num_verts, 0);

      vertex_normals.assign (num_verts, 0);

      for (unsigned t = 0; t < num_triangs; t++)
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
	
      for (unsigned v = 0; v < num_verts; v++)
	if (face_counts[v] > 1)
	  vertex_normals[v] /= face_counts[v];
    }
}

// Add this (or some other ...) surfaces to SPACE
//
void
Mesh::add_to_space (Voxtree &space)
{
  std::cout << "* adding mesh: " << commify (vertices.size ()) << " vertices"
	    << ", " << commify (triangles.size ()) << " triangles"
	    << std::endl;

  for (unsigned i = 0; i < triangles.size(); i++)
    triangles[i].add_to_space (space);
}

// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
