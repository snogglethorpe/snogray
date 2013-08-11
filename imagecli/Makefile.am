# Automake Makefile template for Snogray image command-line handling
# 	library, libsnogimagecli.a
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


noinst_LIBRARIES = libsnogimagecli.a

pkgluamodulesnogray_DATA = image-sampled-output-cmdline.lua

EXTRA_DIST = $(pkgluamodulesnogray_DATA)


libsnogimagecli_a_SOURCES = image-input-cmdline.h			\
	image-sampled-output-cmdline.cc image-sampled-output-cmdline.h	\
	image-scaled-output-cmdline.h


