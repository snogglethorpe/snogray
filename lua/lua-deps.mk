# Makefile fragment for extra compile-time dependencies of libsnoglua.a
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


# Library dependencies of libsnoglua.a
#
libsnoglua_LIBS = $(top_builddir)/snograw/libsnograw.a		\
	$(top_builddir)/lua-util/libsnogluautil.a		\
	$(top_builddir)/liblpeg/liblpeg.a $(liblua_LIBS)
