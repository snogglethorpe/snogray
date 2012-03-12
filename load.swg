# surface.swg -- SWIG interfaces for snogray surfaces
#
#  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "load-ply.h"
#include "load-msh.h"
#if HAVE_LIB3DS
# include "load-3ds.h"
#endif
%}


namespace snogray {


  void load_ply_file (const char *filename, Mesh &mesh,
		      const ValTable &params);

  void load_msh_file (const char *filename, Mesh &mesh,
		      const ValTable &params);

#if HAVE_LIB3DS
  void load_3ds_file (const char *filename, Scene &scene, Camera &camera,
		      const ValTable &params);
  void load_3ds_file (const char *filename, Mesh &mesh,
		      const ValTable &params);
#endif

}
