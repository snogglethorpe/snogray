// lua-util.h -- Snogray Lua utility module "snogray.util"
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LUA_UTIL_H
#define SNOGRAY_LUA_UTIL_H

#include "config.h"

extern "C"
{
#include "lua.h"
}


namespace snogray {


// Return a Lua string containing the entire contents of a file, or
// return false if that can't be done for some reason (it's expected
// that in that case, the caller will then attempt to do the same
// thing using standard lua functions, and determine the error
// itself).
//
// This is basically equivalent to io.open(filename,"r"):read"*a" but
// much more efficient and less likely to thrash the system to death.
//
extern int lua_read_file (lua_State *L);


// Create and return the "snogray.util" module.
//
extern int luaopen_snogray_util (lua_State *L);

}

#endif // SNOGRAY_LUA_UTIL_H
