// lua-setup.cc -- Create a new snogray-specific lua state
//
//  Copyright (C) 2006-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <stdexcept>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#if HAVE_LUAJIT
# include "luajit.h"
#endif
}

#include "lua-funs.h"
#include "funptr-cast.h"
#include "snogpaths.h"
#include "cli/version.h"

#include "lua-setup.h"


using namespace snogray;



// Module "pre-loading"

extern "C" int luaopen_lpeg (lua_State *L);
extern "C" int luaopen_snograw (lua_State *L);

// Wrapper function that calls luaopen_snograw, and then maybe fixes up the
// module state to fix issues with old SWIG versions.
//
static int
luaopen_snograw_fixup (lua_State *L)
{
  int rv = luaopen_snograw (L);
  if (rv)
    {
      // If luaopen_snograw returned a string, that means it put the actual
      // module table in a global variable called "snograw".  Get the value
      // of that table, delete the variable, and return the table instead,
      // to reflect modern Lua module practice.
      //
      if (lua_isstring (L, -1))
	{
	  const char *module_name = lua_tostring (L, -1);
	  lua_getglobal (L, module_name); // get module table from global var
	  lua_insert (L, -2);		  // swap table and module_name
	  lua_pushnil (L);
	  lua_setglobal (L, module_name); // delete global variable
	  lua_pop (L, 1);		  // pop module name
	  // now module table is on the top of the stack
	}
    }
  return rv;
}

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
  { "snogray.snograw", luaopen_snograw_fixup },
  { "lpeg", luaopen_lpeg },
  { 0, 0 }
};


// Lua error-handling

// This is a Lua "panic function": when registered with Lua, it is
// called if any error occurs outside of any pcall.  This one just
// throws a C++ exception.
//
// Note that this may not really do much, as it's possible that Lua
// does not propagate C++ exceptions; in such a case, C++ will just
// call std::terminate.
//
static int
snogray_lua_panic (lua_State *L)
{
  const char *error_msg = lua_tostring (L, -1);
  throw std::runtime_error (error_msg);
}


// setup_module_loader

// Tweak the module system in Lua state L to properly load our modules.
//
static void
setup_lua_module_loader (lua_State *L)
{
  // A small Lua script to set up the module system for loading
  // snogray packages.
  //
  // It expects two arguments:  (1) the directory where we can find
  // installed Lua files, and (2) the name of the file to load to do the
  // module system setup.
  //
  // As this code this has to be executed _before_ we load any modules,
  // we keep it as a C string instead of storing it in a file.
  //
  static const char lua_module_setup_script[] = "\
    local snogray_installed_lua_root, module_setup_file = ... \
    local mod_setup = loadfile (module_setup_file) \
    if mod_setup then \
      mod_setup (nil) \
    else \
      mod_setup = loadfile (snogray_installed_lua_root \
                            ..'/'..module_setup_file) \
      if mod_setup then  \
	mod_setup (snogray_installed_lua_root) \
      else \
	error (module_setup_file..' not found', 0) \
      end \
    end";

  luaL_loadstring (L, lua_module_setup_script);
  lua_pushstring (L, (installed_pkgdatadir () + "/lua").c_str());
  lua_pushstring (L, "module-setup.lua"); // Lua file with module setup code
  lua_call (L, 2, 0);
}


// Lua initialization

// Return a new Lua state setup with our special environment.
//
lua_State *
snogray::new_snogray_lua_state ()
{
  // Do one-time setup of lua environment.

  // Create a new Lua state.  The one created by luaL_newstate uses
  // "realloc" for memory allocation.
  //
  lua_State *L = luaL_newstate ();

  // Set our own "panic function" to throw an exception instead of
  // exiting.
  //
  lua_atpanic (L, snogray_lua_panic);

  // Load standard Lua libraries.
  //
  luaL_openlibs (L);

  // register preloaded modules
  //
  lua_getglobal (L, "package");
  lua_getfield (L, -1, "preload");
  for (preload_module *pm = preloaded_modules; pm->name; pm++)
    {
      lua_pushcfunction (L, pm->loader);
      lua_setfield (L, -2, pm->name);
    }
  lua_pop (L, 1);		// pop package.preload table

  // Add extra functions into snograw table.
  //
  lua_getglobal (L, "require");		 // function
  lua_pushstring (L, "snogray.snograw"); // arg 0
  lua_call (L, 1, 1);			 // call require
  lua_pushcfunction (L, snogray::lua_read_file);
  lua_setfield (L, -2, "read_file");
  lua_pop (L, 1); 		// pop snograw table

  // Setup the module system to load more stuff.
  //
  setup_lua_module_loader (L);

  // Add snogray version string to the "snogray.environ" module.
  //
  lua_getglobal (L, "require");		 // function
  lua_pushstring (L, "snogray.environ"); // arg 0
  lua_call (L, 1, 1);			 // call require
  lua_pushstring (L, snogray_version);
  lua_setfield (L, -2, "version");
  lua_pop (L, 1);			 // pop environ table

  return L;
}
