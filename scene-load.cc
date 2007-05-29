// scene-load.cc -- Loading scene files
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cerrno>
#include <cctype>
#include <fstream>
#include <algorithm>

#include "config.h"

#include "scene.h"
#include "excepts.h"

#if HAVE_LIB3DS
# include "load-3ds.h"
#endif

#if LUA_SCENE_DEF
# include "load-lua.h"
#endif

using namespace snogray;
using namespace std;

void
Scene::load (const string &file_name, const string &fmt, Camera &camera)
{
  string load_fmt = fmt;

  if (load_fmt.empty ())
    { 
      unsigned ext_pos = file_name.find_last_of (".");

      if (ext_pos + 1 >= file_name.length())
	throw runtime_error
	  ("No filename extension to determine scene file format");

      load_fmt = file_name.substr (ext_pos + 1);
    }

  // Make LOAD_FMT lower-case.
  //
  transform (load_fmt.begin(), load_fmt.end(), load_fmt.begin(), ::tolower);

  // First look for formats that want to open the file themselves.
  //
#ifdef HAVE_LIB3DS
  if (load_fmt == "3ds")
    load_3ds_file (file_name, *this, camera);
  else
#endif
#ifdef LUA_SCENE_DEF
  if (load_fmt == "lua")
    load_lua_file (file_name, *this, camera);
  else
#endif
    // Try to open the stream, and then look for formats that want a stream.
    //
    {
      ifstream stream (file_name.c_str());

      if (! stream)
	throw file_error (string ("Cannot open scene file: ")
			  + strerror (errno));

      load (stream, load_fmt, camera);
    }
}

void
Scene::load (istream &stream, const string &fmt, Camera &camera)
{
  if (fmt == "nff" || fmt == "aff")
    load_aff_file (stream, camera);
  else
    throw (runtime_error (string ("Unknown scene file format: ") + fmt));
}

// arch-tag: 6afb7868-7e1a-46de-815c-eed49bdd8609
