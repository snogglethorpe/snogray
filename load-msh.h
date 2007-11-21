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

namespace snogray {

class Mesh;
class MaterialDict;

// Load mesh from a .msh format mesh file into MESH.  Materials are
// filtered through MAT_DICT.
//
extern void load_msh_file (const std::string &filename, Mesh &mesh,
			   const MaterialDict &mat_dict);

}

#endif // __LOAD_MSH_H__
