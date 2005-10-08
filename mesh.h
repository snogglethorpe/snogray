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

#include <fstream>

#include "obj.h"

namespace Snogray {

class Mesh : public Obj
{
public:

  static const enum Error { FILE_FORMAT_ERROR, TOO_MANY_VERTICES_ERROR };

  Mesh (const Material *mat) : Obj (mat), parts (0) { }

  virtual dist_t intersection_distance (const Ray &ray) const;

  virtual Vec normal (const Pos &point, const Vec &eye_dir) const;

  virtual BBox bbox () const;

  static Mesh *read_msh_file (istream stream);

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
