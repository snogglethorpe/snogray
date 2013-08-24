// invoke-driver.cc -- Find and invoke the top-level Lua driver
//
//  Copyright (C) 2011-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "lua-setup.h"
#include "lua-util/lua-val-table.h"

#include "invoke-lua-driver.h"


using namespace snogray;


// Locate the Lua driver file called DRIVER_NAME, and invoke it with a
// Lua table containing the strings in ARGV, which should be arguments
// the process was invoked with.
//
// If an error occurs, print an error message to std::cerr and exit.
//
// ARGV[0] is assumed to be the command name, which is used both for
// error messages, and as a possible location to search for related
// files.  DRIVER_NAME will be searched for in "appropriate places":
// standard installation locations, and relative to the command (for
// running an un-installed binary).
//
void
snogray::invoke_lua_driver (const std::string &driver_name, const char **argv)
{
  const char *prog = argv[0];

  lua_State *L;
  try
    {
      // Get directory from PROG, or use "." if none.
      //
      std::string prog_dir = prog;
      std::string::size_type slash = prog_dir.find_last_of ("/");
      if (slash == std::string::npos)
	prog_dir = ".";
      else
	prog_dir.erase (slash == 0 ? 1 : slash);

      L = new_snogray_lua_state (prog_dir);
    }
  catch (std::exception &exc)
    {
      std::cerr << prog << ": error initializing Lua: " << exc.what ()
		<< std::endl;
      exit (2);
    }

  // Some Lua code to load and run the driver; it expects two
  // arguments: (1) DRIVER_NAME, and (2) a table containing the
  // command-line arguments.
  //
  static char invoke_driver_script[] = "\
    local driver_name, args = ... \
    local environ = require ('snogray.environ') \
    local driver, err = loadfile (environ.lua_dir..'/'..driver_name) \
    if driver then driver (args) else error (err, 0) end";

  // If we encounter an error, the error message gets stored here.
  //
  std::string err_msg;

  // Use our little script to invoke the driver.
  //
  try
    {
      luaL_loadstring (L, invoke_driver_script);

      // Push DRIVER_NAME.
      //
      lua_pushstring (L, driver_name.c_str ());

      // Push a table containing ARGV.
      //
      lua_newtable (L);
      unsigned arg_index = 0;
      while (*argv)
	{
	  lua_pushinteger (L, arg_index++);
	  lua_pushstring (L, *argv++);
	  lua_settable (L, -3);
	}

      // Run the script.
      //
      int err = lua_pcall (L, 2, 0, 0);
      if (err == LUA_ERRRUN)
	err_msg = lua_tostring (L, -1);
      else if (err == LUA_ERRMEM)
	err_msg = "Lua memory allocation error";
      else if (err == LUA_ERRERR)
	err_msg = "Lua error handler error";
    }
  catch (std::exception &exc)
    {
      err_msg = exc.what ();
    }

  if (! err_msg.empty ())
    {
      std::cerr << prog << ": " << err_msg << std::endl;
      exit (4);
    }

  // ... and we're done, clean up our Lua state.
  //
  lua_close (L);
}
