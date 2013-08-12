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
noinst_SCRIPTS = update-version.sh

pkgluamodulesnogray_DATA = cmdlineparser.lua

EXTRA_DIST = update-version.sh $(pkgluamodulesnogray_DATA)

CLEANFILES = version.cc


libsnogcli_a_SOURCES = cmdlineparser.cc cmdlineparser.h		\
	tty-progress.cc tty-progress.h version.cc version.h


################################################################
#
# Version frobbing
#

update_version:
	@$(SHELL) $(srcdir)/update-version.sh "$(PACKAGE_VERSION)" "version.cc"
.PHONY: update_version

version.cc: update_version

# This forces update_version to be run before checking whether the
# library is up-to-date
#
version.h: update_version
