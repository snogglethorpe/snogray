# Makefile fragment for extra compile-time dependencies of libsnogloaders.a
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


# Library dependencies of libsnogloaders.a
#
libsnogloaders_LIBS =

if have_lib3ds
  libsnogloaders_LIBS += $(LIB3DS_LIBS)
endif
