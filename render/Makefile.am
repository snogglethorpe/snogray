# Automake Makefile template for Snogray core rendering library, libsnogrender.a
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


noinst_LIBRARIES = libsnogrender.a

EXTRA_DIST = render.swg


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogrender_a_SOURCES = direct-illum.cc direct-illum.h		\
	direct-integ.h filter-volume-integ.h global-render-state.cc	\
	global-render-state.h grid.cc grid.h integ.h			\
	mis-sample-weight.h path-integ.cc path-integ.h			\
	photon-integ.cc photon-integ.h recursive-integ.cc		\
	recursive-integ.h render-context.cc render-context.h		\
	render-params.h render-stats.cc render-stats.h sample-gen.h	\
	sample-set.cc sample-set.h surface-integ.h volume-integ.h	\
	zero-surface-integ.h
