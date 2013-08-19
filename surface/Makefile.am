# Automake Makefile template for Snogray surface library, libsnogsurf.a
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


noinst_LIBRARIES = libsnogsurf.a

dist_pkgluamodulesnogray_DATA = surface.lua

EXTRA_DIST = surface.swg


libsnogsurf_a_SOURCES = cylinder.cc cylinder.h ellipse.cc ellipse.h	\
	instance.cc instance.h local-primitive.h local-surface.h	\
	mesh.cc mesh.h model.cc model.h primitive.cc primitive.h	\
	sphere.cc sphere.h sphere2.cc sphere2.h surface.cc surface.h	\
	surface-group.cc surface-group.h surface-sampler.cc		\
	surface-sampler.h tessel.cc tessel.h tessel-param.cc		\
	tessel-param.h tessel-sinc.cc tessel-sinc.h tessel-sphere.cc	\
	tessel-sphere.h tessel-torus.cc tessel-torus.h tripar.cc	\
	tripar.h
