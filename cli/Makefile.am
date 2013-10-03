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
include $(top_srcdir)/install-dirs.mk


noinst_LIBRARIES = libsnogcli.a
noinst_SCRIPTS = update-build-info.sh

dist_pkgluamodulesnogray_DATA = cmdlineparser.lua

EXTRA_DIST = update-build-info.sh

CLEANFILES = build-info.cc


libsnogcli_a_SOURCES = cmdlineparser.cc cmdlineparser.h		\
	tty-progress.cc tty-progress.h build-info.cc build-info.h


################################################################
#
# Build-Info frobbing
#

UPD_VERS_ENV_VARS = CC="$(CC)" CC_OPTS="$(AM_CFLAGS)" CXX="$(CXX)"	\
	CXX_OPTS="$(AM_CXXFLAGS)"					\
	CONFIG_STATUS="$(top_builddir)/config.status"
update_build_info:
	@$(UPD_VERS_ENV_VARS) $(SHELL) $(srcdir)/update-build-info.sh \
		"$(PACKAGE_VERSION)" "build-info.cc"
.PHONY: update_build_info

build-info.cc: update_build_info

# This forces update_build_info to be run before checking whether the
# library is up-to-date
#
build-info.h: update_build_info
