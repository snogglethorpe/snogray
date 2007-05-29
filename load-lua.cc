// load-lua.cc -- Load lua scene file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"
#include "excepts.h"

#include "load-lua.h"

#include "swigluarun.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_snograw (lua_State *);
}

using namespace snogray;
using namespace std;

// Global lua state.  All loaded files get the same state, and it is
// kept around after loading (because otherwise objects created in lua
// will be destroyed!?!).
//
lua_State *L = 0;

// Load a lua scene file into SCENE and CAMERA; loads all parts of the
// scene, including lights and the first camera position.
//
void
snogray::load_lua_file (const string &filename, Scene &scene, Camera &camera)
{
  int err = 0;
  string err_str;

  if (! L)
    {
      // Do one-time setup of lua environment.

      L = lua_open();

      luaL_openlibs (L);		// load standard libraries
      luaopen_snograw (L);		// load the wrapped module

      // Mark the low-level "snograw" module as loaded.
      //
      lua_getfield (L, LUA_GLOBALSINDEX, "package");
      lua_getfield (L, -1, "loaded");
      lua_getfield (L, LUA_GLOBALSINDEX, "snograw");
      lua_setfield (L, -2, "snograw");
      lua_pop (L, 1);

      // require ("snogray")
      //
      lua_getfield (L, LUA_GLOBALSINDEX, "require"); // function
      lua_pushstring (L, "snogray");		     // arg 0
      err = lua_pcall (L, 1, 1, 0);		     // call it
      if (err)
	err_str = lua_tostring (L, -1);
      else
	lua_pop (L, 1);
    }

  if (! err)
    {
      // Load the file.

      // Swig types for the stuff we're gonna pass into lua.
      //
      swig_type_info *scene_swig_type
	= SWIG_TypeQuery (L, "snogray::Scene *");
      swig_type_info *camera_swig_type
	= SWIG_TypeQuery (L, "snogray::Camera *");

      // Call "snogray.start_load (filename, scene, camera)" with our
      // scene and camera pointers.
      //
      lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
      lua_getfield (L, -1, "start_load");		    // function
      lua_pushstring (L, filename.c_str ());		    // arg 0
      SWIG_NewPointerObj (L, &scene, scene_swig_type, 0);   // arg 1
      SWIG_NewPointerObj (L, &camera, camera_swig_type, 0); // arg 2
      err = lua_pcall (L, 3, 0, 0);			    // call it
      if (err)
	err_str = lua_tostring (L, -1);

      if (! err)
	{
	  // Load the user's file!  This just constructs a function from the
	  // loaded file, but doesn't actually evaluate it.
	  //
	  err = luaL_loadfile (L, filename.c_str());

	  // Make a new environment to evaluate the file contents in; it
	  // will inherit from "snogray" for convenience.  There are no global
	  // pointers to this table so it and its contents will be garbage
	  // collected after loading.
	  //
	  lua_newtable (L);		    // environ
	  lua_newtable (L);		    // metatable
	  lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
	  lua_setfield (L, -2, "__index");  // metatable.__index = snogray
	  lua_setmetatable (L, -2);	    // setmetatable(environ, metatable)
	  lua_setfenv (L, -2);

	  // After loading, we need to execute the loaded object.
	  //
	  if (err)
	    err_str = "file error";
	  else
	    {
	      err = lua_pcall (L, 0, 0, 0);
	      if (err)
		err_str = lua_tostring (L, -1);
	    }
	}
    }

  // Run the garbage collector to free up any data left around from the
  // user's calculations.
  //
  lua_gc (L, LUA_GCCOLLECT, 0);

  if (err)
    throw file_error (err_str);
}


// arch-tag: 59e28305-b880-41bd-8bf6-7737a67f7e31
