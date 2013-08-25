// lua-vector.h -- Lua wrapper module for C++ vectors
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

#ifndef SNOGRAY_LUA_VECTOR_H
#define SNOGRAY_LUA_VECTOR_H

#include "config.h"

extern "C"
{
#include "lua.h"
}

#include <vector>


namespace snogray {


template<typename T>
struct LuaVecType
{
};


// Functions for a wrapper class, which wraps a std::vector<T> in a
// Lua userdata.
//
template<typename T>
struct LuaVec
{
public:

  // make (SIZE = 0) => VEC
  //
  // Return a new Lua std::vector<T> wrapper with initial size SIZE,
  // defaulting to zero.
  //
  static int make (lua_State *L);

  // If the object at position POS on the Lua stack is one of our Lua
  // wrappers, return a pointer to the wrapped vector, otherwise
  // return zero.
  //
  static std::vector<T> *vec (lua_State *L, int pos);

  // If the object at position POS on the Lua stack is one of our Lua
  // wrappers, return a pointer to the wrapped vector, otherwise
  // signal an error.
  //
  static std::vector<T> *checkvec (lua_State *L, int pos);

  // Register a global metatable for this vector class.  If one has
  // already been registered, nothing is done.
  //
  static void register_metatable (lua_State *L);

private: 

  //
  // See the method definitions in lua-vector.tcc for documentation.
  //

  typedef LuaVecType<T> VT;

  static std::vector<T> *_checkvec (lua_State *L, int pos);

  static int get (lua_State *L);
  static int get1 (lua_State *L);
  static int set (lua_State *L);
  static int set1 (lua_State *L);
  static int add (lua_State *L);
  static int len (lua_State *L);
  static int resize (lua_State *L);

  static int fini (lua_State *L);

  static int tostring (lua_State *L);

  static void register_metatable_entry (lua_State *L,
					const char *name,
					int (*fun) (lua_State *L));
};


// Create and return the "snogray.vector" module.
//
extern int luaopen_snogray_vector (lua_State *L);


}


// Method definitions for LuaVec<T>
//
#include "lua-vector.tcc"


#endif // SNOGRAY_LUA_VECTOR_H
