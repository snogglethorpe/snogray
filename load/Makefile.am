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


# Lua loader directories for various scene/mesh formats.
#
pkglualoaderdir = $(pkgluamodulesnograydir)/loader
pkgluasceneloaderdir = $(pkglualoaderdir)/scene
pkgluameshloaderdir = $(pkglualoaderdir)/mesh

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
dist_pkgluamodulesnogray_DATA = load.lua

# Lua loaders for various scene/mesh formats.
#
dist_pkgluasceneloader_DATA = scene/lua.lua scene/pbrt.lua scene/nff.lua
dist_pkgluameshloader_DATA = mesh/obj.lua mesh/stl.lua mesh/ug.lua

# Try to clean up our extra install directories when uninstalling.
#
uninstall-hook:
	-rmdir $(DESTDIR)$(pkgluameshloaderdir)
	-rmdir $(DESTDIR)$(pkgluasceneloaderdir)
	-rmdir $(DESTDIR)$(pkglualoaderdir)


EXTRA_DIST = load.swg


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogload_a_SOURCES = mesh/load-msh.cc mesh/load-msh.h		\
	mesh/load-ply.cc mesh/load-ply.h mesh/rply.c mesh/rply.h	\
	load-envmap.cc load-envmap.h

if have_lib3ds
  libsnogload_a_SOURCES += scene/load-3ds.cc scene/load-3ds.h
endif

