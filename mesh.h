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

#include "obj.h"

namespace Snogray {

class Mesh : public Obj
{
public:

  Mesh (const Material *mat) : Obj (mat), parts (0) { }

  virtual dist_t intersection_distance (const Ray &ray) const;

  virtual Vec normal (const Pos &point, const Vec &eye_dir) const;

  virtual BBox bbox () const;

private:
  class Part;
  class Triangle;

  Part *parts;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
