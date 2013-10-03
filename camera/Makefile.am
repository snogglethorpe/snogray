# Automake Makefile template for Snogray camera library,
# 	libsnogcamera.a
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


noinst_LIBRARIES = libsnogcamera.a

dist_pkgluamodulesnogray_DATA = camera.lua camera-cmdline.lua

EXTRA_DIST = camera.swg


libsnogcamera_a_SOURCES = camera.cc camera.h
