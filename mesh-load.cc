// mesh-load.cc -- Mesh loading
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <fstream>
#include <cerrno>

#include "config.h"

#include "excepts.h"
#include "string-funs.h"

#ifdef HAVE_LIB3DS
# include "load-3ds.h"
#endif
#include "load-ply.h"

#include "mesh.h"

using namespace snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const string &file_name, const MaterialMap &mat_map,
	    const Xform &xform)
{
  try
    {
      string::size_type ext_pos = file_name.find_last_of (".");

      if (ext_pos == std::string::npos)
	throw runtime_error ("No filename extension to determine mesh format");

      string fmt = downcase (file_name.substr (ext_pos + 1));

      // First look for formats that want to open the file themselves.
      //
#ifdef HAVE_LIB3DS
      if (fmt == "3ds")
	load_3ds_file (file_name, *this, mat_map, xform);
      else
#endif

	if (fmt == "ply")
	  load_ply_file (file_name, *this, mat_map.map (material), xform);

	else
	  // Try to open the stream, and then look for formats that want
	  // a stream.
	  //
	  {
	    ifstream stream (file_name.c_str());

	    if (! stream)
	      throw file_error (string (": ") + strerror (errno));

	    if (fmt == "msh" || fmt == "mesh")
	      load_msh_file (stream, mat_map, xform);
	    else
	      throw (runtime_error ("Unknown mesh file format: " + fmt));
	  }
    }
  catch (runtime_error &err)
    {
      throw runtime_error (file_name + ": " + err.what ());
    }
}


// .msh mesh-file format

void
Mesh::load_msh_file (istream &stream, const MaterialMap &mat_map,
		     const Xform &xform)
{
  char kw[50];

  // .msh files use a right-handed coordinate system by convention, so
  // the mesh will be left-handed only if XFORM reverses the handedness.
  //
  left_handed = xform.reverses_handedness ();

  stream >> kw;
  do
    {
      unsigned base_vert = vertices.size ();

      const Material *mat;
      unsigned num_vertices, num_triangles;

      // See whether a named material or the number of vertices follows.
      //
      if (isdigit (kw[0]))
	{
	  // No, it must be the number of vertices.  Just use a default
	  // material in this case.
	  
	  mat = mat_map.map (material);
	  num_vertices = atoi (kw);
	}
      else
	{
	  // Yes, KW is a material name; map it to a material, and read the
	  // number of vertices from the next line.

	  mat = mat_map.map (kw, material);
	  stream >> num_vertices;
	}

      // The next line should be a triangle count.
      //
      stream >> num_triangles;

      reserve (num_vertices, num_triangles);

      stream >> kw;
      if (strcmp (kw, "vertices") != 0)
	throw bad_format ();

      for (unsigned i = 0; i < num_vertices; i++)
	{
	  Pos pos;

	  stream >> pos.x;
	  stream >> pos.y;
	  stream >> pos.z;

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

	  add_triangle (base_vert + v0i, base_vert + v1i, base_vert + v2i, mat);
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

	      vertex_normals.push_back (MVec (norm * norm_xform).unit ());
	    }

	  stream >> kw;
	}
    }
  while (! stream.eof ());
}


// arch-tag: 50a45108-0f51-4377-9246-7b0bcedf4135
