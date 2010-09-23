// lua-funs.cc -- Functions for use with Lua
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "lauxlib.h"
}

#include "val-table.h"

#include "lua-funs.h"


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
	  // Otherwise, it's a type we can't handle, so ignore it.
	}

       // Pop value from stack, keeps they for the next iteration
       //
       lua_pop (L, 1);
     }

  return 0;
}


// lua_read_file

// Return a Lua string containing the entire contents of a file, or
// return false if that can't be done for some reason (it's expected
// that in that case, the caller will then attempt to do the same
// thing using standard lua functions, and determine the error
// itself).
//
// This is basically equivalent to io.open(filename,"r"):read"*a" but
// much more efficient and less likely to thrash the system to death
// when reading huge files.
//
int
snogray::lua_read_file (lua_State *L)
{
  const char *filename = luaL_checkstring (L, 1);

#if HAVE_UNISTD_H && HAVE_SYS_MMAN_H && HAVE_SYS_STAT_H

  // We have typical unix-style system calls, use them to do the job
  // efficiently.

  int fd = open (filename, O_RDONLY);
  if (fd >= 0)
    {
      struct stat statb;

      if (fstat (fd, &statb) == 0)
	{
	  size_t size = statb.st_size;
	  void *contents = mmap (0, size, PROT_READ, MAP_SHARED, fd, 0);

	  if (contents != MAP_FAILED)
	    {
	      // ugh
	      const char *str_contents
		= static_cast<const char *> (const_cast<const void *> (contents));

#ifdef MADV_SEQUENTIAL
	      madvise (contents, size, MADV_SEQUENTIAL);
#endif

	      // Push a lua string with the result.
	      //
	      lua_pushlstring (L, str_contents, size);

	      munmap (contents, size);
	      close (fd);

	      return 1; // return the string
	    }
	}

      close (fd);
    }

#endif // HAVE_SYS_MMAN_H && HAVE_SYS_STAT_H

  // Return false to indicate to the caller that he should do the job
  // using lua functions.
  //
  lua_pushboolean (L, false);
  return 1;
}
