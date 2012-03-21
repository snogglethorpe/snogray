// load-lua.cc -- Load lua scene file
//
//  Copyright (C) 2006-2008, 2010-2012  Miles Bader <miles@gnu.org>
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
#include "lua-setup.h"

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
snogray::load_lua_file (const string &filename,
			Scene &scene, Camera &camera, ValTable &params)
{
  bool loaded = false;

  if (! L)
    L = new_snogray_lua_state ();

  // Swig types for the stuff we're gonna pass into lua.
  //
  swig_type_info *scene_swig_type = SWIG_TypeQuery (L, "snogray::Scene *");
  swig_type_info *camera_swig_type = SWIG_TypeQuery (L, "snogray::Camera *");

  // Make a Lua copy of PARAMS.
  //
  lua_newtable (L);
  lua_load_from_val_table (L, params);

  // require ("snogray.load") => load module table
  //
  lua_getglobal (L, "require");	      // function
  lua_pushstring (L, "snogray.load"); // arg 0
  lua_call (L, 1, 1);

  // Call "load.scene (filename, scene, camera, camera)" with
  // our scene and camera pointers, and a Lua copy of PARAMS.
  //
  lua_getfield (L, -1, "scene");			// function
  lua_pushstring (L, filename.c_str ());		// arg 0: filename
  SWIG_NewPointerObj (L, &scene, scene_swig_type, 0);   // arg 1: scene
  SWIG_NewPointerObj (L, &camera, camera_swig_type, 0); // arg 2: camera
  lua_pushvalue (L, -6);				// arg 3: params

  lua_call (L, 4, 1);					// do the call
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


// arch-tag: 59e28305-b880-41bd-8bf6-7737a67f7e31
