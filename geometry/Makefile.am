# Automake Makefile template for Snogray geometry library, libsnoggeom.a
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


noinst_LIBRARIES = libsnoggeom.a

dist_pkgluamodulesnogray_DATA = coord.lua transform.lua


libsnoggeom_a_SOURCES = bbox.cc bbox.h bbox-io.cc bbox-io.h coords.h	\
	cone-sample.h cyl-xform.cc cyl-xform.h dir-hist.h		\
	dir-hist-dist.h disk-sample.h frame.h hist-2d.h			\
	hist-2d-dist.cc hist-2d-dist.h local-xform.cc local-xform.h	\
	matrix4.cc matrix4.h matrix4.tcc pos.h pos-io.cc pos-io.h	\
	quadratic-roots.h sphere-isec.h sphere-sample.h			\
	spherical-coords.h tangent-disk-sample.h tripar-isec.h		\
	tuple3.h uv.h uv-io.cc uv-io.h vec.h vec-io.cc vec-io.h		\
	xform.h xform-base.h xform-io.cc xform-io.h
