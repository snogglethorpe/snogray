# Automake Makefile template for local copy of Lua interpreter library
#
#  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

include $(top_srcdir)/config.mk


# Clean up any liblua we've build
#
CLEANFILES = liblua.a


# This directory is optional, and is used only when the Lua library is
# being explicitly built (as opposed to using a system Lua library).
#
if build_lua

noinst_LIBRARIES = liblua.a

# no actual sources, we'll deal
liblua_a_SOURCES = 

# We use two different methods for building liblua.a, depending on
# whether we're using a separate build directory or not, and whether
# the Lua source directory is a subdirectory (and thus is "ours"):
#
# + If building in the source dir, and the Lua source directory is a
#   simple subdirectory of the top-level source dir, we build liblua.a
#   in the Lua source directory and copy it to our top-level
#   directory.  This avoids most assumptions about how make works.
#
# + If using a separate build dir, or if the Lua source directory is
#   "somewhere else", we build liblua.a in the current directory,
#   using the Makefile from the Lua source directory, and VPATH to
#   tell make where to find the source files. This requires that make
#   support at using VPATH to find source files, which seems a
#   reasonable assumption in this case..
#
if build_lua_in_src_dir

liblua.a:
	(cd $(LUA_SRC); $(MAKE) $(AM_MAKEFLAGS) CC='$(CC)' RANLIB='$(RANLIB)' CFLAGS='$(AM_CFLAGS) $(CFLAGS)' liblua.a)
	cp -p $(LUA_SRC)/liblua.a .
	$(RANLIB) $@
clean-local:
	-rm -rf $(LUA_SRC)/liblua.a
mostlyclean-local:
	-rm -rf $(LUA_SRC)/*.o

else !build_lua_in_src_dir

liblua.a:
	$(MAKE) -f $(LUA_SRC)/Makefile $(AM_MAKEFLAGS) CC='$(CC)' RANLIB='$(RANLIB)' CFLAGS='$(AM_CFLAGS) $(CFLAGS)' VPATH=$(LUA_SRC) liblua.a

endif

endif


# Allow specifying a Lua distribution to be included.  If we're already
# building using a local copy of Lua, it is distributed by default.
#
dist-hook:
	if test x"$(LUA_DIST)" != x; then				\
	  cp -pR "$(LUA_DIST)" $(distdir)/lua-dist;			\
	  find $(distdir)/lua-dist -name '*.[oa]' -exec rm {} \; ;	\
	fi
