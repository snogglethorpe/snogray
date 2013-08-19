# Automake Makefile template for Snogray color library, libsnogcolor.a
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


noinst_LIBRARIES = libsnogcolor.a

dist_pkgluamodulesnogray_DATA = color.lua

EXTRA_DIST = color.swg


libsnogcolor_a_SOURCES = color.cc color.h color-io.cc color-io.h	\
	color-math.h tint.h tint-io.cc tint-io.h
