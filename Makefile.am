# Automake Makefile template for snogray
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
include $(top_srcdir)/image/image-deps.mk
include $(top_srcdir)/load/load-deps.mk
include $(top_srcdir)/lua/lua-deps.mk
include $(top_srcdir)/glare/glare-deps.mk


# Targets
#
bin_PROGRAMS = snogray snogcvt snoghilite snogdiff sampleimg

if build_snogbloom
  bin_PROGRAMS += snogbloom
endif


# Library subdirectories
#
SUBDIRS = cli color geometry image intersect light liblpeg liblua	\
	load lua material photon render render-mgr scene snograw	\
	space surface texture util

if build_snogbloom
  SUBDIRS += glare
endif


# Lua files loaded directly.
#
pkglua_DATA = snogray.lua

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
pkgluamodulesnogray_DATA = all-in-one.lua limit-cmdline.lua


# Various files to include in distribution not covered by automatic rules
#
EXTRA_DIST =								\
	autogen.sh config.mk						\
	$(pkglua_DATA) $(pkgluamodulesnogray_DATA) $(pkglualoader_DATA)	\
	README README.building README.issues README.usage README.utils


# Try to clean up the extra subdirectories of $(datadir) we use when
# uninstalling.
#
uninstall-hook:
	-rmdir $(DESTDIR)$(pkglualoaderdir)
	-rmdir $(DESTDIR)$(pkgluamodulesnograydir)
	-rmdir $(DESTDIR)$(pkgluamoduledir)
	-rmdir $(DESTDIR)$(pkgluadir)
	-rmdir $(DESTDIR)$(pkgdatadir)


AM_CPPFLAGS += $(liblua_CFLAGS) $(libsnogimage_CPPFLAGS)


################################################################
#
# Groups of associated libraries
#
RENDER_LIBS = render-mgr/libsnogrendermgr.a render/libsnogrender.a	\
	photon/libsnogphoton.a space/libsnogspace.a			\
	scene/libsnogscene.a material/libsnogmat.a			\
	surface/libsnogsurf.a texture/libsnogtex.a			\
	light/libsnoglight.a intersect/libsnogisec.a			\
	geometry/libsnoggeom.a
LUA_LIBS = lua/libsnoglua.a $(libsnoglua_LIBS)
LOAD_LIBS = load/libsnogload.a $(libsnogload_LIBS)
IMAGE_LIBS = image/libsnogimage.a $(libsnogimage_LIBS)	\
	color/libsnogcolor.a
MISC_LIBS = cli/libsnogcli.a util/libsnogutil.a


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc
snogray_LDADD = $(LUA_LIBS) $(LOAD_LIBS) $(RENDER_LIBS) $(IMAGE_LIBS)	\
	$(MISC_LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

snoghilite_SOURCES = snoghilite.cc
snoghilite_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

if build_snogbloom
  snogbloom_SOURCES = snogbloom.cc
  snogbloom_LDADD = glare/libsnogglare.a $(libsnogglare_LIBS)	\
	  $(IMAGE_LIBS) $(MISC_LIBS)
endif

sampleimg_SOURCES = sampleimg.cc
sampleimg_LDADD = $(RENDER_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)
