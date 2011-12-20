// load.cc -- Snogray scene (etc) loader
//
//  Copyright (C) 2005-2008, 2010-2011  Miles Bader <miles@gnu.org>
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

#if HAVE_LIB3DS
# include "load-3ds.h"
#endif
#if USE_LUA
# include "load-lua.h"
#endif

#include "load.h"


using namespace snogray;


// Load from FILENAME into SCENE and CAMERA.  PARAMS contains various
// global configuration parameters, which can be examined or modified
// by loaders.
//
// FMT is the type of loader; if it is empty, then the file-extension
// of FILENAME is used instead.
//
void
snogray::load_file (const std::string &filename, const std::string &_fmt,
		    Scene &scene, Camera &camera, ValTable &params)
{
  std::cout << "* loading: " << filename << std::endl;

  // Default to using the filename extension to determine the file format.
  //
  std::string fmt = _fmt.empty() ? filename_ext (filename) : _fmt;

#ifdef HAVE_LIB3DS
  if (fmt == "3ds")
    load_3ds_file (filename, scene, camera, params);
  else
#endif

#ifdef USE_LUA
  if (load_lua_file (filename, fmt, scene, camera, params))
      { /* loaded */ }
  else
#endif

    throw (std::runtime_error ("Unknown scene file format \"" + fmt + "\""));
}


// arch-tag: 6afb7868-7e1a-46de-815c-eed49bdd8609
