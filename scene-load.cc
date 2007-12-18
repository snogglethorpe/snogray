// scene-load.cc -- Loading scene files
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

#include "scene.h"


using namespace snogray;
using namespace std;


void
Scene::load (const string &file_name, const string &_fmt, Camera &camera)
{
  std::cout << "* loading scene: " << file_name << std::endl;

  // Default to using the filename extension to determine the file format.
  //
  string fmt = _fmt.empty() ? filename_ext (file_name) : _fmt;

  if (fmt == "nff" || fmt == "aff")
    load_aff_file (file_name, camera);

#ifdef HAVE_LIB3DS
  else if (fmt == "3ds")
    load_3ds_file (file_name, *this, camera);
#endif

#ifdef USE_LUA
  else if (load_lua_file (file_name, fmt, *this, camera))
      { /* loaded */ }
#endif

  else
    throw (runtime_error (string ("Unknown scene file format: ") + fmt));
}


// arch-tag: 6afb7868-7e1a-46de-815c-eed49bdd8609
