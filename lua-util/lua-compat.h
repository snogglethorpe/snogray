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


// Definitions of functions that are new in Lua 5.2 for older versions
// of Lua (or luajit).
//
#if LUA_VERSION_NUM < 502

#undef luaL_newlib
#define luaL_newlib(L, modules_funs)					\
  (lua_createtable (L, 0, sizeof module_funs / sizeof module_funs[0] - 1), \
   luaL_register (L, 0, module_funs))

#define luaL_setmetatable(L, tname) \
  (luaL_getmetatable(L, tname), lua_setmetatable(L, -2))

inline void *
luaL_testudata (lua_State *L, int pos, const char *tname)
{
  void *mem = lua_touserdata (L, pos);
  if (mem && lua_getmetatable (L, pos))
    {
      luaL_getmetatable (L, tname);
      if (! lua_rawequal (L, -1, -2))
	mem = 0;
      lua_pop (L, 2);
    }
  return mem;
}

#endif // LUA_VERSION_NUM < 502


#endif // SNOGRAY_LUA_COMPAT_H
