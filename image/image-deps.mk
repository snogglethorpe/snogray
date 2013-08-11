# Makefile fragment for extra compile-time dependencies of libsnogimage.a
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


# Compiler flags (nominally "cflags", but in practice always
# preprocessor options) needed by libraries we use.
#
# It would be nice to only use these flags when compiling the actual
# object file which needs them -- however automake has very poor support
# for specifying non-global compiler options (even where it can be done,
# automake's implementation has annoying side-effects).
#
libsnogimage_CPPFLAGS = $(libpng_CFLAGS) $(libexr_CFLAGS)	\
	$(libjpeg_CFLAGS) $(libnetpbm_CFLAGS) $(LIB3DS_CFLAGS)


# Library dependencies of libsnogimage.a
#
libsnogimage_LIBS =

if have_libpng
  libsnogimage_LIBS += $(libpng_LIBS)
endif

if have_libexr
  libsnogimage_LIBS += $(libexr_LIBS)
endif

if have_libjpeg
  libsnogimage_LIBS += $(libjpeg_LIBS)
endif

if have_libnetpbm
  libsnogimage_LIBS += $(libnetpbm_LIBS)
endif
