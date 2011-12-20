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

#include "excepts.h"
#include "string-funs.h"

#include "load-ply.h"
#include "load-msh.h"
#ifdef HAVE_LIB3DS
# include "load-3ds.h"
#endif
#if USE_LUA
# include "load-lua.h"
#endif

#include "mesh.h"

using namespace snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const string &file_name)
{
  std::cout << "* loading mesh: " << file_name << std::endl;

  try
    {
      string ext = filename_ext (file_name);

      if (ext == "ply")
	load_ply_file (file_name, *this, ValTable::NONE);
 
      else if (ext == "msh" || ext == "mesh")
	load_msh_file (file_name, *this);

#ifdef HAVE_LIB3DS
      else if (ext == "3ds")
	load_3ds_file (file_name, *this, ValTable::NONE);
#endif

#ifdef USE_LUA
      else if (load_lua_file (file_name, ext, *this))
	{ /* loaded */ }
#endif

      else
	throw (runtime_error ("Unknown mesh file format: " + ext));
    }
  catch (runtime_error &err)
    {
      throw runtime_error (file_name + ": " + err.what ());
    }
}


// arch-tag: 50a45108-0f51-4377-9246-7b0bcedf4135
