// mesh-load.cc -- Mesh loading
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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
#include "string-funs.h"

#include "mesh.h"

using namespace Snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const char *file_name, const Xform &xform,
	    const string &mat_name)
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

	if (strcmp (file_ext, "msh") == 0 || strcmp (file_ext, "mesh") == 0)
	  load_msh_file (stream, xform, mat_name);
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
Mesh::load_msh_file (istream &stream, const Xform &xform,
		     const string &mat_name)
{
  char kw[50];
  bool skip = false;

  if (mat_name.length() > 0)
    stream >> kw;

  // .msh files use a right-handed coordinate system by convention, so
  // the mesh will be left-handed only if XFORM reverses the handedness.
  //
  left_handed = xform.reverses_handedness ();

  do
    {
      if (mat_name.length() > 0)
	skip = (mat_name != kw);

      unsigned base_vert = vertices.size ();

      unsigned num_vertices, num_triangles;
      stream >> num_vertices;
      stream >> num_triangles;

      if (! skip)
	{
	  vertices.reserve (base_vert + num_vertices);
	  triangles.reserve (vertices.size() + num_triangles);
	}

      stream >> kw;
      if (strcmp (kw, "vertices") != 0)
	throw bad_format ();

      for (unsigned i = 0; i < num_vertices; i++)
	{
	  Pos pos;

	  stream >> pos.x;
	  stream >> pos.y;
	  stream >> pos.z;

	  if (! skip)
	    add_vertex (pos * xform);
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

	  if (! skip)
	    add_triangle (base_vert + v0i, base_vert + v1i, base_vert + v2i);
	}

      stream >> kw;

      if (strcmp (kw, "texcoords") == 0)
	{
	  for (unsigned i = 0; i < num_vertices; i++)
	    {
	      float u, v;

	      stream >> u;
	      stream >> v;
	    }

	  stream >> kw;
	}

      if (strcmp (kw, "normals") == 0)
	{
	  if (! skip)
	    vertex_normals.reserve (base_vert + num_vertices);

	  // Calculate a variant of XFORM suitable for transforming
	  // normals.
	  //
	  Xform norm_xform = xform.inverse().transpose();

	  for (unsigned i = 0; i < num_vertices; i++)
	    {
	      Vec norm;

	      stream >> norm.x;
	      stream >> norm.y;
	      stream >> norm.z;

	      if (! skip)
		vertex_normals.push_back ((norm * norm_xform).unit ());
	    }

	  stream >> kw;
	}
    }
  while (! stream.eof ());
}

// arch-tag: 50a45108-0f51-4377-9246-7b0bcedf4135
