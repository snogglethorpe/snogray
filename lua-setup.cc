// lua-setup.cc -- Create a new snogray-specific lua state
//
//  Copyright (C) 2006-2008, 2010-2011  Miles Bader <miles@gnu.org>
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

#include "lua-setup.h"


using namespace snogray;



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


// LuaJIT error-propagation

#if HAVE_LUAJIT

// This is a LuaJIT-specific wrapper function, which catches C++
// exceptions (inside calls to C++ from Lua) and propagates them as
// Lua errors.
//
static int
luajit_exception_wrapper (lua_State *L, lua_CFunction fun)
{
  const char *err_msg = 0;

  try
    {
      // Call FUN; if it successfully returns, so do we.
      //
      return fun (L);
    }
  //
  // Catch various sorts of exceptions from FUN.
  //
  // For things that were obviously thrown in C++, we try to get an
  // error message.
  //
  // Otherwise, the exception may have come from a recursive call into
  // Lua.  Unfortunately, Lua throws an opaque type for errors when
  // compiled in C++, which we can't explicitly check for, but if it
  // did, the error argument will be on the top of the Lua stack in L.
  //
  // So for unknown exceptions, if the Lua stack isn't empty, we just
  // leave it alone, and hope it's the right thing.  If the stack is
  // empty, then the exception clearly didn't come from Lua, and we
  // just use a generic error message.
  //
  catch (const char *str)
    { err_msg = str; }
  catch (std::exception &exc)
    { err_msg = exc.what (); }
  catch (...)
    {
      if (lua_gettop (L) == 0)
	err_msg = "C++ exception"; // not from inferior Lua
    }

  // Call lua_error to propagate the error with ERR_MSG.  If ERR_MSG
  // is zero, then we do nothing, meaning the existing top-of-stack in
  // L is used as the error value.
  //
  if (err_msg)
    lua_pushstring (L, err_msg);

  return lua_error (L);
}

#endif // HAVE_LUAJIT


// Lua error-handling

// This is a Lua "panic function": when registered with Lua, it is
// called if any error occurs outside of any pcall.  This one just
// throws a C++ exception.
//
static int
snogray_lua_panic (lua_State *L)
{
  const char *error_msg = lua_tostring (L, -1);
  throw std::runtime_error (error_msg);
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

  // If we're using LuaJIT, use its "C call wrapper" feature to help
  // propagate exceptions in C++ code called from Lua as Lua errors.
  //
#if HAVE_LUAJIT
  lua_pushlightuserdata (
    L, cast_fun_ptr_to_void_ptr (luajit_exception_wrapper));
  luaJIT_setmode (L, -1, LUAJIT_MODE_WRAPCFUNC|LUAJIT_MODE_ON);
  lua_pop (L, 1);
#endif

  // Load standard Lua libraries.
  //
  luaL_openlibs (L);

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
  lua_call (L, 1, 0);

  // Add extra functions into snograw table.
  //
  // Until recently, Swig-generated Lua modules didn't return the
  // module table from require (as is recommended), but instead stored
  // it into the global variable "snograw", and returned the module
  // name instead.  For compatibility, handle both behaviors, by first
  // calling require, and if it doesn't return a table, looking in the
  // global variable.
  //
  lua_getfield (L, LUA_GLOBALSINDEX, "require"); // function
  lua_pushstring (L, "snograw");		 // arg 0
  lua_call (L, 1, 1);				 // call require
  if (! lua_istable (L, -1))
    {
      // TOS is module name, not module, so grab the global var instead.
      lua_pop (L, 1);		// pop module name
      lua_getfield (L, LUA_GLOBALSINDEX, "snograw");
    }
  lua_pushcfunction (L, snogray::lua_read_file);
  lua_setfield (L, -2, "read_file");
  lua_pop (L, 1); 		// pop snograw table

  return L;
}
