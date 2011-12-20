// load-ply.h -- Load a .ply format mesh file
//
//  Copyright (C) 2006-2008, 2010-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#ifndef SNOGRAY_LOAD_PLY_H
#define SNOGRAY_LOAD_PLY_H


#include <string>

#include "ref.h"


namespace snogray {

class Mesh;
class Material;
class ValTable;


// Load mesh from a .ply format mesh file into MESH.
//
extern void load_ply_file (const std::string &filename, Mesh &mesh,
			   const ValTable &params);


}

#endif // SNOGRAY_LOAD_PLY_H


// arch-tag: 12587fbc-45b0-4843-82b2-5e11bc9b86ec
