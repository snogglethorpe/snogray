# Automake Makefile template for Snogray acceleration-structure
# 	library, libsnogspace.a
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


noinst_LIBRARIES = libsnogspace.a

dist_pkgluamodulesnogray_DATA = accel.lua

EXTRA_DIST = space.swg


################################################################
#
# Snogray acceleration-structure library, libsnogspace.a
#

libsnogspace_a_SOURCES = isec-cache.h octree.cc octree.h	\
	octree-builder.cc octree-node.h space.cc space.h	\
	space-builder.h triv-space.h
