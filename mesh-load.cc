// mesh-load.cc -- Mesh loading
//
//  Copyright (C) 2005-2007, 2010-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "config.h"

#include "load-lua.h"

#include "mesh.h"

using namespace snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const string &file_name)
{
  std::cout << "* loading mesh: " << file_name << std::endl;
  load_lua_file (file_name, *this);
}


// arch-tag: 50a45108-0f51-4377-9246-7b0bcedf4135
