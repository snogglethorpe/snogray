# Automake Makefile template for Snogray "rendering manager" library,
#	libsnogrendermgr.a
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


noinst_LIBRARIES = libsnogrendermgr.a

EXTRA_DIST = render-mgr.swg


libsnogrendermgr_a_SOURCES = render-mgr.cc render-mgr.h		\
	render-packet.h render-pattern.h renderer.cc renderer.h	\
	wire-frame.h

if use_threads
libsnogrendermgr_a_SOURCES += render-queue.cc render-queue.h	\
	render-thread.cc render-thread.h
endif


