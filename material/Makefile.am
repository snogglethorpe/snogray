# Automake Makefile template for Snogray material library, libsnogmat.a
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


noinst_LIBRARIES = libsnogmat.a

dist_pkgluamodulesnogray_DATA = material.lua

EXTRA_DIST = material.swg


libsnogmat_a_SOURCES = bsdf.h cook-torrance.cc cook-torrance.h		\
	cos-dist.h dist.h fresnel.h glass.cc glass.h glow.cc glow.h	\
	hemi-dist.h lambert.cc lambert.h material.cc material.h		\
	material-dict.cc material-dict.h material-wrapper.cc		\
	material-wrapper.h media.cc media.h medium.h mirror.cc		\
	mirror.h norm-glow.cc norm-glow.h phong-dist.h phong.cc		\
	phong.h thin-glass.cc thin-glass.h stencil.cc stencil.h		\
	ward-dist.h xform-material.cc xform-material.h
