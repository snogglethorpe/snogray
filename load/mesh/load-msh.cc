// load-msh.cc -- Load a .msh format mesh file
//
//  Copyright (C) 2006-2008, 2010-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <fstream>
#include <cerrno>
#include <cstring>

#include "config.h"

#include "util/excepts.h"
#include "surface/mesh.h"

#include "load-msh.h"


using namespace snogray;


// Load mesh from a .msh format mesh file into MESH part PART.
//
void
snogray::load_msh_file (const std::string &filename,
			Mesh &mesh, Mesh::part_index_t part,
			const ValTable &)
{
  std::ifstream stream (filename.c_str());
  if (! stream)
    throw file_error (std::string (": ") + strerror (errno));

  // .msh files use a right-handed coordinate system by convention.
  //
  mesh.left_handed = false;

  char kw[50];

  stream >> kw;
  do
    {
      unsigned base_vert = mesh.num_vertices ();

      unsigned num_vertices, num_triangles;

      // Get the number of vertices.
      //
      num_vertices = atoi (kw);

      // The next line should be a triangle count.
      //
      stream >> num_triangles;

      mesh.reserve_vertices (num_vertices);

      stream >> kw;
      if (strcmp (kw, "vertices") != 0)
	throw bad_format ();

      for (unsigned i = 0; i < num_vertices; i++)
	{
	  Pos pos;

	  stream >> pos.x;
	  stream >> pos.y;
	  stream >> pos.z;

	  mesh.add_vertex (pos);
	}

      stream >> kw;
      if (strcmp (kw, "triangles") != 0)
	throw bad_format ();

      std::vector<Mesh::vert_index_t> vert_indices;
      vert_indices.reserve (num_triangles * 3);
      for (unsigned i = 0; i < num_triangles; i++)
	for (unsigned v = 0; v < 3; v++)
	{
	  unsigned vert_index;
	  stream >> vert_index;
	  vert_indices.push_back (vert_index);
	}
      mesh.add_triangles (part, vert_indices, base_vert);

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
	  mesh.reserve_normals ();

	  for (unsigned i = 0; i < num_vertices; i++)
	    {
	      Vec norm;

	      stream >> norm.x;
	      stream >> norm.y;
	      stream >> norm.z;

	      mesh.add_normal (base_vert + i, norm.unit ());
	    }

	  stream >> kw;
	}
    }
  while (! stream.eof ());
}
