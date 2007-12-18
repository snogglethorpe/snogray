// load-ply.h -- Load a .ply format mesh file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#ifndef __LOAD_PLY_H__
#define __LOAD_PLY_H__

#include <string>

namespace snogray {

class Mesh;
class Material;

// Load mesh from a .ply format mesh file into MESH.  If non-zero the
// material MAT will be used for triangles loaded (otherwise MESH's
// default material will be used).
//
extern void load_ply_file (const std::string &filename, Mesh &mesh,
			   const Material *mat = 0);

}

#endif /* __LOAD_PLY_H__ */

// arch-tag: 12587fbc-45b0-4843-82b2-5e11bc9b86ec
