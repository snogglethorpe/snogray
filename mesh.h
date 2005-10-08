// mesh.h -- Mesh object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MESH_H__
#define __MESH_H__

#include <stdexcept>
#include <istream>

#include "primary-obj.h"

namespace Snogray {

class Mesh : public PrimaryObj
{
public:

  Mesh (const Material *mat)
    : PrimaryObj (mat),
      vertices (0), vertex_normals (0), num_vertices (0),
      triangles (0), num_triangles (0)
  { }
  Mesh (const Material *mat, const char *file_name)
    : PrimaryObj (mat),
      vertices (0), vertex_normals (0), num_vertices (0),
      triangles (0), num_triangles (0)
  { load (file_name); }
  virtual ~Mesh ();

  // Add this (or some other ...) objects to SPACE
  //
  virtual void add_to_space (Voxtree &space);

  // For loading mesh from any file-type (automatically determined)
  //
  void load (const char *file_name);

  // For loading mesh from .msh file
  //
  void load_msh_file (std::istream &stream);

  //
  void compute_vertex_normals ();

private:

  class Triangle;

  // A list of vertices used in this part.
  Pos *vertices;
  Vec *vertex_normals;
  unsigned num_vertices;

  // A vector of Mesh::Triangle objects that use this part.
  Triangle *triangles;
  unsigned num_triangles;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
