# Automake Makefile template for LPeg library, liblpeg.a
#
#  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

include $(top_srcdir)/config.mk


noinst_LIBRARIES = liblpeg.a

EXTRA_DIST = lpeg.html


AM_CPPFLAGS += $(liblua_CFLAGS)


liblpeg_a_SOURCES = lpcap.c lpcap.h lpcode.c lpcode.h lpprint.c	\
	lpprint.h lptree.c lptree.h lptypes.h lpvm.c lpvm.h


