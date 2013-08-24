// lua-val-table.cc -- Functions for converting between ValTables and Lua tables
//
//  Copyright (C) 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

extern "C"
{
#include "lauxlib.h"
}

#include "util/val-table.h"

#include "lua-val-table.h"


using namespace snogray;


// lua_get_val_table_entries

// Copy all entries in VAL_TABLE into the Lua table on the top of the
// stack.  Returns nothing.
//
int
snogray::lua_load_from_val_table (lua_State *L, const ValTable &val_table)
{
  luaL_checktype (L, -1, LUA_TTABLE);

  for (ValTable::const_iterator i = val_table.begin ();
       i != val_table.end (); ++i)
    {
      const std::string &key = i->first;
      const Val &val = i->second;

      switch (val.type)
	{
	case Val::STRING:
	default:  // try to handle unexpected types
	  lua_pushstring (L, val.as_string ().c_str ());
	  break;
	case Val::INT:
	  lua_pushinteger (L, val.as_int ());
	  break;
	case Val::UINT:
	  // Assuming lua_Number is double, then this probably
	  // preserves the sign even in cases where the high bit is
	  // set (in which case using lua_pushinteger might end up
	  // pushing something negative).
	  //
	  lua_pushnumber (L, val.as_uint ());
	  break;
	case Val::FLOAT:
	  lua_pushnumber (L, val.as_float ());
	  break;
	case Val::BOOL:
	  lua_pushboolean (L, val.as_bool ());
	  break;
	case Val::TABLE:
	  // Try to re-use an existing subtable, but if some
	  // non-table is stored in this entry, just overwrite it with
	  // a new empty table.
	  //
	  lua_getfield (L, -1, key.c_str ());
	  if (! lua_istable (L, -1))
	    {
	      lua_pop (L, 1);
	      lua_newtable (L);
	    }
	  lua_load_from_val_table (L, val.as_table ());
	  break;
	}

      lua_setfield (L, -2, key.c_str ());
    }

  return 0;
}


// snogray::lua_put_table_entries

// Copy all entries from the Lua table on the top of the stack into
// VAL_TABLE.  Any entries with non-string keys are ignored (i.e., the
// table's array-part), as are any entries with values that cannot be
// stored into a VAL_TABLE.  Returns nothing.
//
int
snogray::lua_store_into_val_table (lua_State *L, ValTable &val_table)
{
  luaL_checktype (L, -1, LUA_TTABLE);

  // The key nil tells lua_next to give us the first entry.
  //
  lua_pushnil (L);

  while (lua_next (L, -2))
    {
      if (lua_isstring (L, -2))
	{
	  const char *key = lua_tostring (L, -2);

	  if (lua_isstring (L, -1))
	    val_table.set (key, lua_tostring (L, -1));
	  else if (lua_isboolean (L, -1))
	    val_table.set (key, lua_toboolean (L, -1));
	  else if (lua_isnumber (L, -1))
	    {
	      // In the normal case where lua_Number is a double, none
	      // of the types that can be stored in a ValTable is a
	      // perfect superset, so try to find one that works for
	      // the actual value.  If nothing else works, we store as
	      // a float (which might lose some precision).
	      //
	      lua_Number num = lua_tonumber (L, -1);
	      int num_as_int = static_cast<int> (num);
	      int num_as_uint = static_cast<unsigned> (num);
	      if (static_cast<lua_Number> (num_as_int) == num)
		val_table.set (key, num_as_int);
	      else if (static_cast<lua_Number> (num_as_uint) == num)
		val_table.set (key, num_as_uint);
	      else
		val_table.set (key, static_cast<float> (num));
	    }
	  else if (lua_istable (L, -1))
	    {
	      // If the existing entry called KEY isn't a table, just
	      // delete it.
	      //
	      Val *subt_val = val_table.get (key);
	      if (subt_val && subt_val->type != Val::TABLE)
		val_table.erase (key);
	      lua_store_into_val_table (L, val_table.writable_subtable (key));
	    }
	  // Otherwise, it's a type we can't handle, so ignore it.
	}

       // Pop value from stack, keeps they for the next iteration
       //
       lua_pop (L, 1);
     }

  return 0;
}
