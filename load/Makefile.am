# Automake Makefile template for Snogray format-specific loader
#	library, libsnogload.a
#
#  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
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
include $(top_srcdir)/image/image-deps.mk


noinst_LIBRARIES = libsnogload.a

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
pkgluamodulesnogray_DATA = load.lua

# Lua loaders for various scene/mesh formats.
#
pkglualoader_DATA = load-lua.lua load-nff.lua load-obj.lua	\
	load-pbrt.lua load-stl.lua load-ug.lua

EXTRA_DIST = load.swg $(pkgluamodulesnogray_DATA) $(pkglualoader_DATA)


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogload_a_SOURCES = load-msh.cc load-msh.h load-ply.cc	\
	load-ply.h rply.c rply.h load-envmap.cc load-envmap.h

if have_lib3ds
  libsnogload_a_SOURCES += load-3ds.cc load-3ds.h
endif

