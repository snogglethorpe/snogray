# Automake Makefile template for Snogray glare library, libsnogglare.a
#
#  Copyright (C) 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

include $(top_srcdir)/config.mk
include $(top_srcdir)/image/image-deps.mk


noinst_LIBRARIES = libsnogglare.a


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


libsnogglare_a_SOURCES = add-glare.cc add-glare.h glare-psf.h	\
	photopic-glare-psf.cc photopic-glare-psf.h
