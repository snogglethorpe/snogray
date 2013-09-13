// lua-vector.tcc -- Lua wrapper module for C++ vectors
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

#ifndef SNOGRAY_LUA_VECTOR_TCC
#define SNOGRAY_LUA_VECTOR_TCC

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-compat.h"


namespace snogray {



// Type-specific adapter classes

// vector<int> ops
//
template<>
struct LuaVecType<int>
{
  static void get (lua_State *L, const std::vector<int> &vec,
		   lua_Unsigned idx)
  {
    lua_pushinteger (L, vec[idx]);
  }
  static void set (lua_State *L, int pos,
		   std::vector<int> &vec, lua_Unsigned idx)
  {
    vec[idx] = luaL_checkinteger (L, pos);
  }
  static void add (lua_State *L, int pos,
		   std::vector<int> &vec)
  {
    vec.push_back (luaL_checkinteger (L, pos));
  }
  static const char *name ()
  {
    return "vector<int>";
  }
};

// vector<unsigned> ops
//
template<>
struct LuaVecType<unsigned>
{
  static void get (lua_State *L, const std::vector<unsigned> &vec,
		   lua_Unsigned idx)
  {
    lua_pushunsigned (L, vec[idx]);
  }
  static void set (lua_State *L, int pos,
		   std::vector<unsigned> &vec, lua_Unsigned idx)
  {
    vec[idx] = luaL_checkunsigned (L, pos);
  }
  static void add (lua_State *L, int pos,
		   std::vector<unsigned> &vec)
  {
    vec.push_back (luaL_checkunsigned (L, pos));
  }
  static const char *name ()
  {
    return "vector<unsigned>";
  }
};

// vector<float> ops
//
template<>
struct LuaVecType<float>
{
  static void get (lua_State *L, const std::vector<float> &vec,
		   lua_Unsigned idx)
  {
    lua_pushnumber (L, vec[idx]);
  }
  static void set (lua_State *L, int pos,
		   std::vector<float> &vec, lua_Unsigned idx)
  {
    vec[idx] = luaL_checknumber (L, pos);
  }
  static void add (lua_State *L, int pos,
		   std::vector<float> &vec)
  {
    vec.push_back (luaL_checknumber (L, pos));
  }
  static const char *name ()
  {
    return "vector<float>";
  }
};


// Exported functions

// make (SIZE = 0) => VEC
//
// Return a new Lua std::vector<T> wrapper with initial size SIZE,
// defaulting to zero.
//
template<typename T>
int
LuaVec<T>::make (lua_State *L)
{
  lua_Unsigned init_len = luaL_optunsigned (L, 1, 0);
  void *mem = lua_newuserdata (L, sizeof (std::vector<T>));
  new (mem) std::vector<T> (init_len);
  luaL_setmetatable (L, VT::name ());
  return 1;
}

// If the object at position POS on the Lua stack is one of our Lua
// wrappers, return a pointer to the wrapped vector, otherwise return
// zero.
//
template<typename T>
std::vector<T> *
LuaVec<T>::vec (lua_State *L, int pos)
{
  void *mem = luaL_checkudata (L, pos, VT::name ());
  return static_cast<std::vector<T> *> (mem);
}

// If the object at position POS on the Lua stack is one of our Lua
// wrappers, return a pointer to the wrapped vector, otherwise
// signal an error.
//
template<typename T>
std::vector<T> *
LuaVec<T>::checkvec (lua_State *L, int pos)
{
  std::vector<T> *rval = vec (L, pos);
  luaL_argerror (L, pos, "invalid vector object");
  return rval;
}

// Register a global metatable for this vector class.  If one has
// already been registered, nothing is done.
//
template<typename T>
void
LuaVec<T>::register_metatable (lua_State *L)
{
  if (luaL_newmetatable (L, VT::name ()))
    {
      register_metatable_entry (L, "get", get);
      register_metatable_entry (L, "__index", get1);
      register_metatable_entry (L, "set", set);
      register_metatable_entry (L, "__newindex", set1);
      register_metatable_entry (L, "__len", len);
      register_metatable_entry (L, "add", add);
      register_metatable_entry (L, "clear", clear);
      register_metatable_entry (L, "resize", resize);
      register_metatable_entry (L, "reserve", reserve);
      register_metatable_entry (L, "__gc", fini);
      register_metatable_entry (L, "__tostring", tostring);

      lua_pop (L, 1); // pop metatable
    }
}


// Internal helper functions
//
// The first Lua upvalue must be the shared metatable for this vector
// class (which is established by register_metatable).
//

// If the object at position POS on the Lua stack is one of our Lua
// wrappers, return a pointer to the wrapped vector, otherwise
// signal an error.
//
template<typename T>
std::vector<T> *
LuaVec<T>::_checkvec (lua_State *L, int pos)
{
  void *mem = lua_touserdata(L, pos);
  if (!mem
      || !lua_getmetatable (L, pos)
      || !lua_rawequal (L, -1, lua_upvalueindex (1)))
    luaL_argerror (L, pos, "invalid vector object");
  lua_pop (L, 1); // get rid of metatable
  return static_cast<std::vector<T> *> (mem);
}

// The top of the stack must be a table.  Add a C closure containing
// FUN and a single upvalue pointing to the table, to the table with a
// key of NAME.
//
template<typename T>
void
LuaVec<T>::register_metatable_entry (lua_State *L,
				     const char *name,
				     int (*fun) (lua_State *L))
{
  lua_pushvalue (L, -1); // upvalue holding metatable
  lua_pushcclosure (L, fun, 1);
  lua_setfield (L, -2, name);
}


// Operators
//
// The first Lua upvalue must be the shared metatable for this vector
// class (which is established by register_metatable).
//

// get (VEC, INDEX, COUNT) => VAL1, ...
//
// Return COUNT values from VEC starting at location INDEX.
//
template<typename T>
int
LuaVec<T>::get (lua_State *L)
{
  const std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned idx = luaL_checkunsigned (L, 2) - 1;
  lua_Unsigned count = luaL_optunsigned (L, 3, 1);
  size_t size = vec->size ();
  if (idx + count > size)
    luaL_argerror (L, 3, "index out of range");
  for (lua_Unsigned i = 0; i < count; i++)
    VT::get (L, *vec, idx + i);
  return count;
}  

// get1 (VEC, INDEX) => VAL
//
// Return the value at location INDEX in VEC.
//
// INDEX may also be a string, in which case the corresponding
// metatable entry is returned instead (this is used to implement
// vector methods).
//
template<typename T>
int
LuaVec<T>::get1 (lua_State *L)
{
  const std::vector<T> *vec = _checkvec (L, 1);
  if (lua_type (L, 2) == LUA_TSTRING)
    {
      lua_getmetatable (L, 1);
      lua_pushvalue (L, 2);
      lua_rawget (L, -2);
    }
  else
    {
      lua_Unsigned idx = luaL_checkunsigned (L, 2) - 1;
      size_t size = vec->size ();
      if (idx >= size)
	luaL_argerror (L, 2, "index out of range");
      VT::get (L, *vec, idx);
    }
  return 1;
}  

// set (VEC, INDEX, VAL1, ...)
//
// Store VAL1, ... into VEC starting at location INDEX.
//
template<typename T>
int
LuaVec<T>::set (lua_State *L)
{
  std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned idx = luaL_checkunsigned (L, 2) - 1;
  lua_Unsigned count = lua_gettop (L) - 2;
  size_t size = vec->size ();
  if (idx > size)
    luaL_argerror (L, 2, "index out of range");
  for (lua_Unsigned i = 0; i < count; i++)
    if (idx + i != size)
      VT::set (L, 3 + i, *vec, idx + i); // normal set
    else
      { // extend
	VT::add (L, 3 + i, *vec);
	size++;
      }
  return 0;
}

// set (VEC, INDEX, VAL)
//
// Store VAL into VEC at location INDEX.
//
template<typename T>
int
LuaVec<T>::set1 (lua_State *L)
{
  std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned idx = luaL_checkunsigned (L, 2) - 1;
  size_t size = vec->size ();
  if (idx > size)
    luaL_argerror (L, 2, "index out of range");
  if (idx == size)
    VT::add (L, 3, *vec);
  else
    VT::set (L, 3, *vec, idx);
  return 0;
}

// add (VEC, VAL1, ...)
//
// Increase the size of VEC by the number of values VAL1, ..., and
// store those values into the newly added locations.
//
template<typename T>
int
LuaVec<T>::add (lua_State *L)
{
  std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned count = lua_gettop (L) - 1;
  for (lua_Unsigned i = 0; i < count; i++)
    VT::add (L, 2 + i, *vec);
  return 0;
}

// len (VEC)
//
// Return the size of VEC.
//
template<typename T>
int
LuaVec<T>::len (lua_State *L)
{
  lua_pushunsigned (L, _checkvec (L, 1)->size ());
  return 1;
}

// clear (VEC)
//
// Set the size of VEC to zero elements.
//
template<typename T>
int
LuaVec<T>::clear (lua_State *L)
{
  _checkvec (L, 1)->clear ();
  return 0;
}

// resize (VEC, SIZE)
//
// Set the size of VEC to SIZE elements.
//
template<typename T>
int
LuaVec<T>::resize (lua_State *L)
{
  std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned size = luaL_checkunsigned (L, 2);
  vec->resize (size);
  return 0;
}

// reserve (VEC, SIZE)
//
// Make sure VEC has enough memory allocated to hold SIZE elements.
// This does not change the visible size of VEC or affect its
// contents, but may make future size increase more efficient.
//
template<typename T>
int
LuaVec<T>::reserve (lua_State *L)
{
  std::vector<T> *vec = _checkvec (L, 1);
  lua_Unsigned size = luaL_checkunsigned (L, 2);
  vec->reserve (size);
  return 0;
}

// fini (VEC)
//
// Call the C++ vector destructor on VEC's vector, freeing any
// resources it has allocated.  No other operations may be performed
// on the vector following this.
//
template<typename T>
int
LuaVec<T>::fini (lua_State *L)
{
  _checkvec (L, 1)->~vector ();
  return 0;
}

// tostring (VEC)
//
// Return a simple string representation of VEC.
//
template<typename T>
int
LuaVec<T>::tostring (lua_State *L)
{
  lua_pushstring (L, VT::name ());
  return 1;
}


} // namespace snogray

#endif // SNOGRAY_LUA_VECTOR_TCC
