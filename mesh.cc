// mesh.cc -- Mesh surface			-*- coding: utf-8 -*-
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
	   const Tessel::MaxErrCalc &max_err,
	   bool smooth)
{
  // Do the tessellation
  //
  Tessel tessel (tessel_fun, max_err);

  unsigned base_vert = vertices.size ();

  tessel.get_vertices (vertices);

  std::vector<unsigned> tri_vert_indices;
  tessel.get_triangle_vertex_indices (tri_vert_indices);

  unsigned num_tris = tri_vert_indices.size () / 3;

  triangles.reserve (triangles.size() + num_tris);

  unsigned tvi_num = 0;
  for (unsigned t = 0; t < num_tris; t++)
    {
      add_triangle (base_vert + tri_vert_indices[tvi_num + 0],
		    base_vert + tri_vert_indices[tvi_num + 1],
		    base_vert + tri_vert_indices[tvi_num + 2]);
      tvi_num += 3;
    }

  if (smooth)
    {
      tessel.get_vertex_normals (vertex_normals);
      if (vertex_normals.size() <= base_vert)
	compute_vertex_normals ();
    }
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


// Mesh triangles

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
Mesh::Triangle::intersection_distance (const Ray &ray, IsecParams &isec_params,
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
  Vec edge1 = v(1) - v(0);
  Vec edge2 = v(2) - v(0);

  /* begin calculating determinant - also used to calculate U parameter */
  Vec pvec = ray.dir.cross (edge2);

  /* if determinant is near zero, ray lies in plane of triangle */
  double det = edge1.dot (pvec);

  if (det > -Eps && det < Eps)
    return 0;

  double inv_det = 1.0 / det;

  /* calculate distance from vert0 to ray origin */
  Vec tvec = ray.origin - v(0);

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
Mesh::Triangle::intersect_info (const Ray &ray, const IsecParams &isec_params)
  const
{
  // We first use the real "raw" normal to determine if this is a back
  // face or not (for back-face determination the normal doesn't need
  // to be a unit vector, so it is only made a unit vector later,
  // perhaps after replacing it w ith the interpolated normal).
  //
  Vec norm = raw_normal_unscaled ();
  bool back = norm.dot (ray.dir) > 0;

  // Point of intersection.
  //
  Pos point = ray.end ();

  // Now if we're using normal interpolation, calculate the
  // interpolated normal.
  //
  if (! mesh.vertex_normals.empty ())
    {
      norm =  vnorm(0) * (1 - isec_params.u - isec_params.v);
      norm += vnorm(1) * isec_params.u;
      norm += vnorm(2) * isec_params.v;

      // If the interpolated normal is pointing in (roughly) the same
      // direction as RAY, it means the interpolation has interpolated past
      // a virtual tangent point on the surface.  In this case there's not
      // much we can do -- it will look ugly no matter what -- but try to
      // keep things as sane as possible by clamping the normal at the
      // point where it's perpendicular to RAY.
      //
      if (!back && norm.dot (ray.dir) > 0)
	{
	  Vec cx = norm.cross (ray.dir);
	  norm = ray.dir.cross (cx);
	}
    }

  return Intersect (ray, this, point, norm.unit(), back);
}

// Return true if RAY would hit the back of this surface.
//
bool
Mesh::Triangle::back (const Ray &ray) const
{
  return raw_normal_unscaled().dot (ray.dir) > 0;
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
