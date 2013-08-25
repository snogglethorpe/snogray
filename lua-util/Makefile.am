# Automake Makefile template for Snogray Lua utility library, libsnogluautil.a
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
include $(top_srcdir)/install-dirs.mk


noinst_LIBRARIES = libsnogluautil.a

dist_pkgluamodulesnogray_DATA = file.lua filename.lua lpeg-utils.lua	\
	  string-fill.lua string.lua swig.lua sys.lua table.lua

EXTRA_DIST = lua-cvt-num-vector.swg lua-val-table.swg


AM_CPPFLAGS += $(liblua_CFLAGS)


libsnogluautil_a_SOURCES = lua-compat.h lua-util.cc lua-util.h		\
	lua-val-table.cc lua-val-table.h lua-vector.cc lua-vector.h	\
	lua-vector.tcc
