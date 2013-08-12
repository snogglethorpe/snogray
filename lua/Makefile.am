# Automake Makefile template for Snogray Lua support library, libsnoglua.a
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
include $(top_srcdir)/lua-install-dirs.mk


noinst_LIBRARIES = libsnoglua.a

pkglua_DATA = module-setup.lua

EXTRA_DIST = $(pkglua_DATA)


AM_CPPFLAGS += $(liblua_CFLAGS)


libsnoglua_a_SOURCES = invoke-lua-driver.cc invoke-lua-driver.h	\
	lua-funs.cc lua-funs.h lua-setup.cc lua-setup.h
