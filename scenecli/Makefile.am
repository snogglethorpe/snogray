# Automake Makefile template for Snogray scene command-line handling
# 	library, libsnogscenecli.a
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
include $(top_srcdir)/image/image-deps.mk


noinst_LIBRARIES = libsnogscenecli.a

pkgluamodulesnogray_DATA = camera-cmdline.lua scene-cmdline.lua

EXTRA_DIST = $(pkgluamodulesnogray_DATA)


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogscenecli_a_SOURCES = scene-cmdline.cc scene-cmdline.h
