// load-lua.cc -- Load lua scene file
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
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

#include "swigluarun.h"
#include "lua-funs.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_snograw (lua_State *);
}

#include "load-lua.h"


using namespace snogray;
using namespace std;


// Global lua state.  All calls to Lua loaders use the same state.
//
// This state can be destroyed after loading by calling the
// cleanup_load_lua_state function after loading (but see the comment
// for that function for some caveats).
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
extern "C" int luaopen_snograw (lua_State *L);

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
  { "snograw", luaopen_snograw },
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

  // require ("snogray")
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "require"); // function
  lua_pushstring (L, "snogray");		 // arg 0
  do_call (L, 1, 0);

  // Add extra functions into snograw table.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "snograw");
  lua_pushcfunction (L, lua_read_file);
  lua_setfield (L, -2, "read_file");
  lua_pop (L, 1); 		// pop snograw table
}


// Lua cleanup

// Cleanup and free all global Lua state.
//
// Note that this function only has an effect if the SWIG version we
// used to generate our Lua interface has the "disown" feature (as
// indicated by the autoconf'd HAVE_SWIG_DISOWN macro).  If SWIG
// doesn't have that feature, then this function does nothing, and our
// global Lua state will never be freed.
//
// [The SWIG "disown" feature is used to pass ownership of certain
// objects from Lua to the snogray core.  If SWIG _doesn't_ have the
// "disown" feature, then destroying the Lua state can end up
// destroying objects which are still referenced other places in
// snogray.]
//
void
snogray::cleanup_load_lua_state ()
{
#if HAVE_SWIG_DISOWN
  if (L)
    {
      lua_close (L);
      L = 0;
    }
#endif
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
			Scene &scene, Camera &camera, ValTable &params)
{
  bool loaded = false;

  if (! L)
    setup_lua ();

  // Swig types for the stuff we're gonna pass into lua.
  //
  swig_type_info *scene_swig_type = SWIG_TypeQuery (L, "snogray::Scene *");
  swig_type_info *camera_swig_type = SWIG_TypeQuery (L, "snogray::Camera *");

  // Make a Lua copy of PARAMS.
  //
  lua_newtable (L);
  lua_load_from_val_table (L, params);

  // Call "snogray.load_scene (filename, scene, camera, camera)" with
  // our scene and camera pointers, and a Lua copy of PARAMS.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
  lua_getfield (L, -1, "load_scene");			// function
  lua_pushstring (L, filename.c_str ());		// arg 0: filename
  lua_pushstring (L, fmt.c_str ());			// arg 1: format
  SWIG_NewPointerObj (L, &scene, scene_swig_type, 0);   // arg 2: scene
  SWIG_NewPointerObj (L, &camera, camera_swig_type, 0); // arg 3: camera
  lua_pushvalue (L, -7);				// arg 4: params

  do_call (L, 5, 1);					// do the call
  loaded = lua_toboolean (L, -1);			// get result

  // Pop the result and the "snogray" table off the stack
  //
  lua_pop (L, 2);

  // Now update PARAMS from the possibly-changed Lua version.
  //
  lua_store_into_val_table (L, params);

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
			Mesh &mesh)
{
  bool loaded = false;

  if (! L)
    setup_lua ();

  // Swig types for the stuff we're gonna pass into lua.
  //
  swig_type_info *mesh_swig_type = SWIG_TypeQuery (L, "snogray::Mesh *");

  // Call "snogray.load_mesh (filename, mesh, camera)" with our
  // mesh and camera pointers.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "snogray");
  lua_getfield (L, -1, "load_mesh");			// function
  lua_pushstring (L, filename.c_str ());		// arg 0
  lua_pushstring (L, fmt.c_str ());			// arg 1
  SWIG_NewPointerObj (L, &mesh, mesh_swig_type, 0);	// arg 2

  do_call (L, 4, 1);					// do the call
  loaded = lua_toboolean (L, -1);			// get result

  // Run the garbage collector to free up any data left around from the
  // user's calculations.
  //
  lua_gc (L, LUA_GCCOLLECT, 0);

  return loaded;
}


// arch-tag: 59e28305-b880-41bd-8bf6-7737a67f7e31
