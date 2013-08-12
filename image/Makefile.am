# Automake Makefile template for Snogray image I/O library, libsnogimage.a
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
include $(srcdir)/image-deps.mk


noinst_LIBRARIES = libsnogimage.a

pkgluamodulesnogray_DATA = image.lua

EXTRA_DIST = image-deps.mk image.swg $(pkgluamodulesnogray_DATA)


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogimage_a_SOURCES = image.h image-box-filt.h image-byte-vec.cc	\
	image-byte-vec.h image-dispatch.cc image-filter-conv.h		\
	image-filter.cc image-filter.h image-gauss-filt.h		\
	image-input.h image-io.cc image-io.h image-mitchell-filt.h	\
	image-sampled-output.cc image-sampled-output.h			\
	image-scaled-output.cc image-scaled-output.h image-pfm.cc	\
	image-pfm.h image-rgbe.cc image-rgbe.h image-tga.cc		\
	image-tga.h image-triangle-filt.h recover-image.cc		\
	recover-image.h tuple-adaptor.h tuple-matrix.cc			\
	tuple-matrix.h tuple-matrix.tcc

if have_libpng
  libsnogimage_a_SOURCES += image-png.cc image-png.h
endif

if have_libexr
  libsnogimage_a_SOURCES += image-exr.cc image-exr.h
endif

if have_libjpeg
  libsnogimage_a_SOURCES += image-jpeg.cc image-jpeg.h
endif

if have_libnetpbm
  libsnogimage_a_SOURCES += image-ppm.cc image-ppm.h
endif
