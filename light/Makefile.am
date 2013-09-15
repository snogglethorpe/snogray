# Automake Makefile template for Snogray light library, libsnoglight.a
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
include $(top_srcdir)/image/image-deps.mk


noinst_LIBRARIES = libsnoglight.a

dist_pkgluamodulesnogray_DATA = light.lua

EXTRA_DIST = light.swg


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnoglight_a_SOURCES = envmap-light.cc envmap-light.h far-light.cc	\
	far-light.h image-sum.h light.h light-sampler.h			\
	point-light.cc point-light.h sphere-light-sampler.cc		\
	sphere-light-sampler.h surface-light-sampler.cc			\
	surface-light-sampler.h
