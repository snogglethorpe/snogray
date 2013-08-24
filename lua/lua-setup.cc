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

#include "lua-util/lua-util.h"
#include "util/snogpaths.h"
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
  // Module name of snograw module used in its SWIG definition file
  // (note that this may be different the name used in the Lua module
  // system).
  //
  const char *module_name = "snograw";

  // Remember the initial TOS position, so we can see if it changes.
  //
  int initial_stack_len = lua_gettop (L);

  // Initialize the actual snograw module.
  //
  int rv = luaopen_snograw (L);

  if (rv)
    {
      // In new versions of SWIG, the module init function simply
      // returns the module table, which is "modern" Lua module
      // practice, and exactly what we want.
      //
      // In old versions of SWIG, on the other hand, the module init
      // function returns nothing, and instead puts the module table
      // in a global variable with the same name as the module.
      //
      // We need to detect the latter situation, and fix it up to look
      // like the former, by grabbing the contents of the global
      // variable (the module table), pushing it on the stack, and
      // deleting the global variable.
      //
      if (lua_gettop (L) == initial_stack_len)
	{
	  lua_getglobal (L, module_name); // get module table from global var
	  lua_pushnil (L);
	  lua_setglobal (L, module_name); // delete global variable
	}

      // Now the module table is on the top of the stack (regardless
      // of what version of SWIG was used).
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
  { "snogray.util", luaopen_snogray_util },
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

// Tweak the module system in Lua state L to properly load our
// modules.  UNINSTALLED_DIR is a directory to search for lua source
// files in uninstalled-mode; if it is empty uninstalled-mode isn't
// used.
//
static void
setup_lua_module_loader (lua_State *L, const std::string &uninstalled_dir)
{
  // A small Lua script to set up the module system for loading
  // snogray packages.
  //
  // It expects three arguments:  (1) a directory to search for an
  // uninstalled snogray distribution, (2) the directory where we can
  // find installed Lua files, and (3) the name of the file to load to
  // do the module system setup.
  //
  // If (1) is non-nil and the snogray module-loader Lua source file
  // given by (3) can be found relative to the directory given by (1),
  // then it is assumed we are running in "uninstalled mode", and that
  // all snogray Lua files will be found in their source-tree
  // locations relative to that directory.  Otherwise it is assumed we
  // are running in "installed mode", and that all snogray Lua files
  // can be found in their installed location relative to the
  // directory given by (2).
  //
  // As this code this has to be executed _before_ we load any modules,
  // we keep it as a C string instead of storing it in a file.
  //
  static const char lua_module_setup_script[] = "\
    local snogray_uninstalled_root, snogray_installed_lua_root, module_setup_file = ... \
    local mod_setup = \
      (snogray_uninstalled_root \
       and loadfile (snogray_uninstalled_root..'/lua/'..module_setup_file)) \
    if mod_setup then \
      mod_setup (false, snogray_uninstalled_root) \
    else \
      mod_setup = loadfile (snogray_installed_lua_root \
                            ..'/'..module_setup_file) \
      if mod_setup then  \
	mod_setup (true, snogray_installed_lua_root) \
      else \
	error (module_setup_file..' not found', 0) \
      end \
    end";

  luaL_loadstring (L, lua_module_setup_script);
  if (uninstalled_dir.empty ())
    lua_pushnil (L);		// don't try uninstalled mode
  else
    lua_pushstring (L, uninstalled_dir.c_str ());
  lua_pushstring (L, (installed_pkgdatadir () + "/lua").c_str());
  lua_pushstring (L, "module-setup.lua"); // Lua file with module setup code
  lua_call (L, 3, 0);
}


// Lua initialization

// Return a new Lua state setup with our special environment.
// UNINSTALLED_DIR is a directory to search for lua source files in
// uninstalled-mode; if it is empty uninstalled-mode isn't used.
//
lua_State *
snogray::new_snogray_lua_state (const std::string &uninstalled_dir)
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

  // Setup the module system to load more stuff.
  //
  setup_lua_module_loader (L, uninstalled_dir);

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
