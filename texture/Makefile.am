# Automake Makefile template for Snogray texture library, libsnogtex.a
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


noinst_LIBRARIES = libsnogtex.a

dist_pkgluamodulesnogray_DATA = texture.lua

EXTRA_DIST = texture.swg


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogtex_a_SOURCES = arith-tex.cc arith-tex.h arith-tex.tcc		\
	check-tex.h cmp-tex.cc cmp-tex.h cmp-tex.tcc coord-tex.h	\
	cubemap.cc cubemap.h envmap.h grey-tex.h intens-tex.h		\
	interp-tex.h matrix-linterp.h matrix-tex.cc matrix-tex.h	\
	matrix-tex.tcc misc-map-tex.h perlin.cc perlin.h perlin-tex.h	\
	perturb-tex.h rescale-tex.h spheremap.cc spheremap.h tex.h	\
	tex-coords.h worley.cc worley.h worley-tex.h xform-tex.h


