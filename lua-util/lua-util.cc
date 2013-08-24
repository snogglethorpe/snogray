// lua-util.h -- Lua utility module "snogray.util"
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

#include "lua-compat.h"

#include "lua-util.h"


using namespace snogray;


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


// Module creation

static luaL_Reg module_funs[] = {
  { "read_file", lua_read_file },
  { 0, 0 }
};

// Create and return the "snogray.util" module.
//
int
snogray::luaopen_snogray_util (lua_State *L)
{
  luaL_newlib (L, module_funs);
  return 1;
}
