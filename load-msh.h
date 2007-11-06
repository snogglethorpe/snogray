// load-msh.h -- Load a .msh format mesh file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#ifndef __LOAD_MSH_H__
#define __LOAD_MSH_H__

#include <string>

#include "xform.h"

namespace snogray {

class Mesh;
class MaterialMap;

// Load mesh from a .msh format mesh file into MESH.  Geometry is first
// transformed by XFORM, and materials filtered through MAT_MAP.
//
extern void load_msh_file (const std::string &filename, Mesh &mesh,
			   const MaterialMap &mat_map, const Xform &xform);

}

#endif // __LOAD_MSH_H__
