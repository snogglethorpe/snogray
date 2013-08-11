# Automake Makefile template for Snogray command-line-interface
# 	library, libsnogcli.a
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


noinst_LIBRARIES = libsnogcli.a

pkgluamodulesnogray_DATA = cmdlineparser.lua

EXTRA_DIST = $(pkgluamodulesnogray_DATA)


libsnogcli_a_SOURCES = cmdlineparser.cc cmdlineparser.h		\
	tty-progress.cc tty-progress.h version.cc version.h
