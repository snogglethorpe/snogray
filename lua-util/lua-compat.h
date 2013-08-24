// lua-compat.h -- Lua backward-compatibility stuff
//
//  Copyright (C) 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LUA_COMPAT_H
#define SNOGRAY_LUA_COMPAT_H

#include "config.h"

extern "C"
{
#include "lauxlib.h"
}


#if LUA_VERSION_NUM < 502

// Version of Lua before 5.2 don't have luaL_newlib, so add our own version
//
#undef luaL_newlib
#define luaL_newlib(L, modules_funs)					\
  (lua_createtable (L, 0, sizeof module_funs / sizeof module_funs[0] - 1), \
   luaL_register (L, 0, module_funs))

#define luaL_setmetatable(L, tname) \
  (luaL_getmetatable(L, tname), lua_setmetatable(L, -2))

#endif // LUA_VERSION_NUM < 502


#endif // SNOGRAY_LUA_COMPAT_H
