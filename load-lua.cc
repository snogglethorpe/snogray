// load-lua.cc -- Load lua scene file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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



// Does a Lua "pcall" and if an error is returned, throws a C++ exception.
// Otherwise, the pcall results are left on the stack.
//
static void
do_call (lua_State *L, int nargs, int nres)
{
  int err = lua_pcall (L, nargs, nres, 0);
  if (err)
    {
      string err_str = lua_tostring (L, -1);
      throw file_error (err_str);
    }
}


// Module "pre-loading"

extern "C" int luaopen_lpeg (lua_State *L);

struct preload_module
{
  const char *name;
  lua_CFunction loader;
};

// A list of modules which are statically linked into our executable and
// should be preloaded (which allows Lua's require mechanism to find
// them).
//
static preload_module preloaded_modules[] = {
  { "lpeg", luaopen_lpeg },
  { 0, 0 }
};


// Lua initialization

// Setup Lua environment, and initialze the global variable L.
//
static void
setup_lua ()
{
  // Do one-time setup of lua environment.

  L = lua_open();

  luaL_openlibs (L);		// load standard libraries
  luaopen_snograw (L);		// load the wrapped module

  // register preloaded modules
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "package");
  lua_getfield (L, -1, "preload");
  for (preload_module *pm = preloaded_modules; pm->name; pm++)
    {
      lua_pushcfunction (L, pm->loader);
      lua_setfield (L, -2, pm->name);
    }
  lua_pop (L, 1);		// pop package.preload table

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
  lua_pushstring (L, "snogray");		 // arg 0
  do_call (L, 1, 0);
}


// Lua scene loading

// If FILENAME is a format that has a Lua scene loader, load the file named
// FILENAME into SCENE and CAMERA using Lua, and return true; if FILENAME
// is unrecogized, return false.  If an error occurs during loading, an
// exception is thrown.  Loads all parts of the scene, including lights and
// the first camera position.
//
bool
snogray::load_lua_file (const string &filename, const std::string &fmt,
			Scene &scene, Camera &camera)
{
  bool loaded = false;

  if (! L)
    setup_lua ();

  // Swig types for the stuff we're gonna pass into lua.
  //
  swig_type_info *scene_swig_type = SWIG_TypeQuery (L, "snogray::Scene *");
  swig_type_info *camera_swig_type = SWIG_TypeQuery (L, "snogray::Camera *");

  // Call "snogray.load_scene (filename, scene, camera)" with our
  // scene and camera pointers.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
  lua_getfield (L, -1, "load_scene");			// function
  lua_pushstring (L, filename.c_str ());		// arg 0
  lua_pushstring (L, fmt.c_str ());			// arg 1
  SWIG_NewPointerObj (L, &scene, scene_swig_type, 0);   // arg 2
  SWIG_NewPointerObj (L, &camera, camera_swig_type, 0); // arg 3

  do_call (L, 4, 1);					// do the call
  loaded = lua_toboolean (L, -1);			// get result

  // Run the garbage collector to free up any data left around from the
  // user's calculations.
  //
  lua_gc (L, LUA_GCCOLLECT, 0);

  return loaded;
}


// Lua mesh loading

// If FILENAME is a format that has a Lua mesh loader, load the file named
// FILENAME into MESH using Lua, and return true; if FILENAME is
// unrecogized, return false.  If an error occurs during loading, an
// exception is thrown.
//
bool
snogray::load_lua_file (const string &filename, const std::string &fmt,
			Mesh &mesh, const MaterialDict &mat_dict)
{
  bool loaded = false;

  if (! L)
    setup_lua ();

  // Swig types for the stuff we're gonna pass into lua.
  //
  swig_type_info *mesh_swig_type = SWIG_TypeQuery (L, "snogray::Mesh *");
  swig_type_info *matm_swig_type = SWIG_TypeQuery (L, "snogray::MaterialDict *");

  // Call "snogray.load_mesh (filename, mesh, camera)" with our
  // mesh and camera pointers.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
  lua_getfield (L, -1, "load_mesh");			// function
  lua_pushstring (L, filename.c_str ());		// arg 0
  lua_pushstring (L, fmt.c_str ());			// arg 1
  SWIG_NewPointerObj (L, &mesh, mesh_swig_type, 0);	// arg 2
  SWIG_NewPointerObj (L, &mat_dict, matm_swig_type, 0);	// arg 3

  do_call (L, 4, 1);					// do the call
  loaded = lua_toboolean (L, -1);			// get result

  // Run the garbage collector to free up any data left around from the
  // user's calculations.
  //
  lua_gc (L, LUA_GCCOLLECT, 0);

  return loaded;
}


// arch-tag: 59e28305-b880-41bd-8bf6-7737a67f7e31
