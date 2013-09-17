# surface.swg -- SWIG interfaces for snogray surfaces
#
#  Copyright (C) 2011-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "load/mesh/load-ply.h"
#include "load/mesh/load-msh.h"
#include "load/scene/load-3ds.h"
%}


%{
// To avoid the need for users to have SWIG available, we always
// export the same interface, and define error stubs here for
// functions which are found not to be available at configure time.

#if !HAVE_LIB3DS
  // Error stubs for functions defined in "load-3ds.h"

  void snogray::load_3ds_file (const std::string &,
       			       SurfaceGroup &scene, Camera &,
			       const ValTable &)
  {
    throw std::runtime_error ("load_3ds_file function not available");
  }
  
  void snogray::load_3ds_file (const std::string &, Mesh &, const ValTable &)
  {
    throw std::runtime_error ("load_3ds_file function not available");
  }
#endif
%}


namespace snogray {


  void load_ply_file (const char *filename, Mesh &mesh,
		      const ValTable &params);

  void load_msh_file (const char *filename, Mesh &mesh,
		      const ValTable &params);

  void load_3ds_file (const char *filename,
       		      SurfaceGroup &scene, Camera &camera,
		      const ValTable &params);
  void load_3ds_file (const char *filename, Mesh &mesh,
		      const ValTable &params);


}
