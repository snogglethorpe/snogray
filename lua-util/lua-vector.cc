// lua-vector.cc -- Lua wrapper module for C++ vectors
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

#include "config.h"

#include "lua-compat.h"

#include "lua-vector.h"


using namespace snogray;


static luaL_Reg module_funs[] = {
  { "int", LuaVec<int>::make },
  { "float", LuaVec<float>::make },
  { "unsigned", LuaVec<unsigned>::make },
  { 0, 0 }
};

// Create and return the "snogray.vector" module.
//
int
snogray::luaopen_snogray_vector (lua_State *L)
{
  luaL_newlib (L, module_funs);
  LuaVec<int>::register_metatable (L);
  LuaVec<float>::register_metatable (L);
  LuaVec<unsigned>::register_metatable (L);
  return 1;
}
