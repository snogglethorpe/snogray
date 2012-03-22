# vector.swg -- Lua table -> std::vector conversion
#
#  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

// For handling variable-length array arguments, passed as std::vectors.
//
%{
#include <vector>

// Reads the contents of the table at INDEX on the Lua stack into a
// vector of numbers, and returns a heap-allocated vector of type
// std::vector<T>*.  Lua numbers will be cast into the type T.  If the
// object at INDEX is not a table, or the table contains non-numbers,
// zero is returned instead.
//
template<class T>
std::vector<T> *
SWIG_read_numeric_vector (lua_State *L, int index)
{
  if (! lua_istable (L, index))
    return 0;			// error

  std::vector<T> *vec = new std::vector<T> ();

#if LUA_VERSION_NUM >= 502
  size_t len = lua_rawlen (L, index);
#else
  size_t len = lua_objlen (L, index);
#endif
  vec->reserve (len);

  for (size_t i = 0; i < len; i++)
    {
      lua_rawgeti (L, index, i + 1);
      if (! lua_isnumber (L, -1))
	{
	  lua_pop (L, 1);
	  delete vec;
	  return 0;		// error
	}
      vec->push_back (T (lua_tonumber (L, -1)));
      lua_pop (L, 1);
    }

  return vec;
}
%}

%define SWIG_TYPEMAP_NUM_VECTOR(T)
%typemap(in) const std::vector<T> &INPUT
%{	$1 = SWIG_read_numeric_vector<T> (L,$input);
	if (!$1) SWIG_fail; %}
%typemap(freearg) const std::vector<T> &INPUT
%{	delete $1; %}
%enddef
