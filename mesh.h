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
      vertices (0), num_vertices (0),
      triangles (0), num_triangles (0)
  { }
  Mesh (const Material *mat, const char *file_name)
    : PrimaryObj (mat),
      vertices (0), num_vertices (0),
      triangles (0), num_triangles (0)
  { load (file_name); }

  virtual dist_t intersection_distance (const Ray &ray) const;

  virtual Vec normal (const Pos &point, const Vec &eye_dir) const;

  virtual BBox bbox () const;

  // For loading mesh from any file-type (automatically determined)
  void load (const char *file_name);

  // For loading mesh from .msh file
  void load_msh_file (std::istream &stream);

protected:

  // We use a 16-bit index for triangle vertex references.
  // If there are more than 2^16 vertices, we'll have to use extra mesh
  // objects to divide things up.
  //
  typedef unsigned short vertex_index_t;

private:

  class Triangle;

  // A list of vertices used in this part.
  Pos *vertices;
  unsigned num_vertices;

  // A vector of Mesh::Triangle objects that use this part.
  Triangle *triangles;
  unsigned num_triangles;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
