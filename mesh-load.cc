// mesh-load.cc -- Mesh loading
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include "excepts.h"
#include "string-funs.h"

#include "load-ply.h"
#include "load-msh.h"
#ifdef HAVE_LIB3DS
# include "load-3ds.h"
#endif

#include "mesh.h"

using namespace snogray;
using namespace std;


// Generic mesh-file loading

void
Mesh::load (const string &file_name, const MaterialMap &mat_map,
	    const Xform &xform)
{
  try
    {
      string ext = filename_ext (filename);

      if (ext == "ply")
	load_ply_file (file_name, *this, mat_map.map (material), xform);
 
      else if (ext == "msh" || ext == "mesh")
	load_msh_file (file_name, *this, mat_map, xform);

#ifdef HAVE_LIB3DS
      else if (ext == "3ds")
	load_3ds_file (file_name, *this, mat_map, xform);
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
