// load-msh.cc -- Load a .msh format mesh file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <fstream>
#include <cerrno>
#include <cstring>

#include "config.h"

#include "excepts.h"
#include "mesh.h"
#include "material-dict.h"

#include "load-msh.h"


using namespace snogray;


// Load mesh from a .msh format mesh file into MESH.  Materials are
// filtered through MAT_DICT.
//
void
snogray::load_msh_file (const std::string &filename, Mesh &mesh,
			const MaterialDict &mat_dict)
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

      const Material *mat;
      unsigned num_vertices, num_triangles;

      // See whether a named material or the number of vertices follows.
      //
      if (isdigit (kw[0]))
	{
	  // No, it must be the number of vertices.  Just use a default
	  // material in this case.
	  
	  mat = mat_dict.get_default ();
	  num_vertices = atoi (kw);
	}
      else
	{
	  // Yes, KW is a material name; map it to a material, and read the
	  // number of vertices from the next line.

	  mat = mat_dict.get (kw);
	  stream >> num_vertices;
	}

      // The next line should be a triangle count.
      //
      stream >> num_triangles;

      mesh.reserve (num_vertices, num_triangles);

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

      for (unsigned i = 0; i < num_triangles; i++)
	{
	  unsigned v0i, v1i, v2i;

	  stream >> v0i;
	  stream >> v1i;
	  stream >> v2i;

	  mesh.add_triangle (base_vert + v0i, base_vert + v1i, base_vert + v2i,
			     mat);
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
