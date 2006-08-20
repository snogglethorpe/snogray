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
#include <cerrno>

#include "config.h"

#include "excepts.h"
#include "string-funs.h"

#ifdef HAVE_LIB3DS
# include "load-3ds.h"
#endif
#include "load-ply.h"

#include "mesh.h"

using namespace Snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const string &file_name, const Xform &xform, const string &mat_name)
{
  try
    {
      unsigned ext_pos = file_name.find_last_of (".");

      if (ext_pos == std::string::npos)
	throw runtime_error ("No filename extension to determine mesh format");

      string fmt = downcase (file_name.substr (ext_pos + 1));

      // First look for formats that want to open the file themselves.
      //
#ifdef HAVE_LIB3DS
      if (fmt == "3ds")
	load_3ds_file (file_name, *this, xform);
      else
#endif

	if (fmt == "ply")
	  load_ply_file (file_name, *this, xform);

	else
	  // Try to open the stream, and then look for formats that want
	  // a stream.
	  //
	  {
	    ifstream stream (file_name.c_str());

	    if (! stream)
	      throw file_error (string (": ") + strerror (errno));

	    if (fmt == "msh" || fmt == "mesh")
	      load_msh_file (stream, xform, mat_name);
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
