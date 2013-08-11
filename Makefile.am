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


# Targets
#
bin_PROGRAMS = snogray snogcvt snoghilite snogdiff sampleimg
noinst_PROGRAMS =

if build_snogbloom
  bin_PROGRAMS += snogbloom
endif


# Targets we don't want for whatever reason.  Automake bitches if we
# simply remove them from the above target variables, so we
# "conditionally" add them back here.
#
if never

# snogsamp is broken now
bin_PROGRAMS += snogsamp

# hemint is rarely used, so don't compile it
noinst_PROGRAMS += hemint

endif


SUBDIRS = color geometry image imagecli intersect light material	\
	space surface texture


include lua-install-dirs.mk


# Lua files loaded directly.
#
pkglua_DATA = module-setup.lua snogray.lua

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
pkgluamodulesnogray_DATA = all-in-one.lua camera.lua			\
	camera-cmdline.lua cmdlineparser.lua file.lua filename.lua	\
	limit-cmdline.lua load.lua lpeg-utils.lua render-cmdline.lua	\
	render.lua scene.lua scene-cmdline.lua string-fill.lua		\
	string.lua swig.lua sys.lua table.lua

# Lua loaders for various scene/mesh formats.
#
pkglualoader_DATA = load-lua.lua load-nff.lua load-obj.lua	\
	load-pbrt.lua load-stl.lua load-ug.lua


# Internal libraries; not installed
#
noinst_LIBRARIES = libsnogcli.a libsnogloaders.a libsnoglua.a		\
	libsnogphoton.a libsnograw.a libsnogrdrive.a libsnogrender.a	\
	libsnogscene.a libsnogscenecli.a libsnogutil.a liblpeg.a


# Various files to include in distribution not covered by automatic rules
#
EXTRA_DIST =								\
	config.mk							\
	$(pkglua_DATA)							\
	$(pkgluamodulesnogray_DATA) $(pkglualoader_DATA)		\
	README README.building README.issues README.usage		\
	README.utils							\
	camera.swg common.swg load.swg					\
	rdrive.swg render.swg scene.swg snograw.swg			\
	util.swg val-table.swg vector.swg				\
	update-version.sh						\
	autogen.sh							\
	lpeg.html							\
	stats.txt spd.txt

noinst_SCRIPTS = update-version.sh


# Try to clean up the extra subdirectories of $(datadir) we use when
# uninstalling.
#
uninstall-hook:
	-rmdir $(DESTDIR)$(pkglualoaderdir)
	-rmdir $(DESTDIR)$(pkgluamodulesnograydir)
	-rmdir $(DESTDIR)$(pkgluamoduledir)
	-rmdir $(DESTDIR)$(pkgluadir)
	-rmdir $(DESTDIR)$(pkgdatadir)


# Allow specifying a Lua distribution to be included.  If we're already
# building using a local copy of Lua, it is distributed by default.
#
dist-hook:
	if test x"$(LUA_DIST)" != x; then			\
	  cp -pR "$(LUA_DIST)" $(distdir)/lua;			\
	  find $(distdir)/lua -name '*.[oa]' -exec rm {} \; ;	\
	fi


# Some extra stuff that needs cleaning up
#
CLEANFILES = liblua.a snograw.so version.cc snogpaths-data.h

# We only clean up "snograw_lua_wrap.cc" if it's possible to
# regenerate it.
#
if have_swig
CLEANFILES += snograw_lua_wrap.cc
endif


include $(top_srcdir)/config.mk
include $(top_srcdir)/image/image-deps.mk


AM_CPPFLAGS += $(libsnogimage_CPPFLAGS)


################################################################
#
# Version frobbing
#

update_version:
	@$(SHELL) $(srcdir)/update-version.sh "$(PACKAGE_VERSION)" "version.cc"
.PHONY: update_version

config.h: update_version


################################################################
#
# Snogray core rendering library, libsnogrender.a
#

libsnogrender_a_SOURCES = direct-illum.cc direct-illum.h		\
	direct-integ.h filter-volume-integ.h global-render-state.cc	\
	global-render-state.h grid.cc grid.h integ.h			\
	mis-sample-weight.h path-integ.cc path-integ.h			\
	photon-integ.cc photon-integ.h recursive-integ.cc		\
	recursive-integ.h render-context.cc render-context.h		\
	render-params.h render-stats.cc render-stats.h sample-gen.h	\
	sample-set.cc sample-set.h surface-integ.h volume-integ.h	\
	zero-surface-integ.h


################################################################
#
# Snogray photon-mapping library, libsnogphoton.a
#

libsnogphoton_a_SOURCES = photon.h photon-eval.cc photon-eval.h		\
	photon-map.cc photon-map.h photon-shooter.cc photon-shooter.h


################################################################
#
# Snogray rendering driver library, libsnogrdrive.a
#

libsnogrdrive_a_SOURCES = render-cmdline.h render-mgr.cc render-mgr.h	\
	render-packet.h render-pattern.h renderer.cc renderer.h		\
	wire-frame.h

if use_threads
libsnogrdrive_a_SOURCES += render-queue.cc render-queue.h	\
	render-thread.cc render-thread.h
endif


################################################################
#
# Snogray scene-representation library, libsnogscene.a
#

libsnogscene_a_SOURCES = camera.cc camera.h scene.cc scene.h


################################################################
#
# Snogray command-line-interface library, libsnogcli.a
#

libsnogcli_a_SOURCES = cmdlineparser.cc cmdlineparser.h		\
	tty-progress.cc tty-progress.h version.cc version.h


################################################################
#
# Snogray scene command-line handling library, libsnogscenecli.a
#

libsnogscenecli_a_SOURCES =  scene-cmdline.cc scene-cmdline.h


################################################################
#
# Snogray format-specific loader library
#

libsnogloaders_a_SOURCES = load-msh.cc load-msh.h load-ply.cc	\
	load-ply.h rply.c rply.h load-envmap.cc load-envmap.h

# Library dependencies of libsnogloaders.a
#
libsnogloaders_LIBS =

if have_lib3ds
  libsnogloaders_a_SOURCES += load-3ds.cc load-3ds.h
  libsnogloaders_LIBS += $(LIB3DS_LIBS)
endif


################################################################
#
# Snogray Lua support library, libsnoglua.a
#

libsnoglua_a_SOURCES = invoke-lua-driver.cc invoke-lua-driver.h	\
	lua-funs.cc lua-funs.h lua-setup.cc lua-setup.h

# Library dependencies of libsnoglua.a
#
libsnoglua_LIBS = libsnograw.a liblpeg.a $(liblua_LIBS)


################################################################
#
# LPeg library.  This library isn't so common, so we just use our own
# copy.
#

liblpeg_a_SOURCES = lpcap.c lpcap.h lpcode.c lpcode.h lpprint.c		\
	lpprint.h lptree.c lptree.h lptypes.h lpvm.c lpvm.h


################################################################
#
# Snogray general utility library, libsnogutil.a
#

libsnogutil_a_SOURCES = compiler.h cond-var.h excepts.h file-funs.cc	\
	file-funs.h float-excepts-guard.h freelist.cc freelist.h	\
	funptr-cast.h gaussian-filter.h globals.cc globals.h grab.h	\
	interp.h llist.h least-squares-fit.h matrix.h matrix.tcc	\
	matrix-funs.h matrix-funs.tcc matrix-io.h mempool.cc		\
	mempool.h mutex.h nice-io.cc nice-io.h num-cores.cc		\
	num-cores.h pool.h progress.h radical-inverse.h random.h	\
	random-boost.h random-std.h random-rand.h random-tr1.h ref.h	\
	rusage.h snogassert.cc snogassert.h snogmath.h snogpaths.cc	\
	snogpaths.h string-funs.cc string-funs.h thread.h threading.h	\
	threading-boost.h threading-std.h timeval.cc timeval.h		\
	val-table.cc unique-ptr.h val-table.h


snogpaths.o: snogpaths-data.h

# automagically-computed paths
#
snogpaths-data.h: Makefile
	$(snogpaths_verbose)( \
	  echo '// this file is automatically generated'; \
	  echo '#define SNOGRAY_PREFIX "$(prefix)"'; \
	  echo '#define SNOGRAY_BINDIR "$(bindir)"'; \
	  echo '#define SNOGRAY_PKGDATADIR "$(pkgdatadir)"' ) > "$@"

# automake "silent-rules" hook for snogpaths-data.h
#
snogpaths_verbose = $(snogpaths_verbose_$(V))
snogpaths_verbose_ = $(snogpaths_verbose_$(AM_DEFAULT_VERBOSITY))
snogpaths_verbose_0 = @echo "  PATHS   " $@;


################################################################
#
# Extra library configuration
#

LIBS += $(MUDFLAP:-f%=-l%)


################################################################
#
# Groups of associated libraries
#
CORE_LIBS = libsnogrender.a libsnogphoton.a space/libsnogspace.a	\
	libsnogscene.a material/libsnogmat.a surface/libsnogsurf.a	\
	texture/libsnogtex.a light/libsnoglight.a			\
	intersect/libsnogisec.a geometry/libsnoggeom.a
LUA_LIBS = libsnoglua.a $(libsnoglua_LIBS)
LOADER_LIBS = libsnogloaders.a $(libsnogloaders_LIBS)
IMAGE_LIBS = imagecli/libsnogimagecli.a image/libsnogimage.a	\
	$(libsnogimage_LIBS) color/libsnogcolor.a
MISC_LIBS = libsnogcli.a libsnogutil.a


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc recover-image.cc recover-image.h
snogray_LDADD = $(LUA_LIBS) libsnogrdrive.a $(LOADER_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) \
	$(MISC_LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = $(LOADER_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

snoghilite_SOURCES = snoghilite.cc
snoghilite_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

if build_snogbloom
  snogbloom_SOURCES = snogbloom.cc add-glare.cc add-glare.h	\
	glare-psf.h photopic-glare-psf.cc photopic-glare-psf.h
  snogbloom_LDADD = $(IMAGE_LIBS) $(MISC_LIBS) $(libfftw3_LIBS)
endif

hemint_SOURCES = hemint.cc
hemint_LDADD = libsnogutil.a

sampleimg_SOURCES = sampleimg.cc
sampleimg_LDADD = $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)

###############################################################
#
# Local copy of lua distribution (optional)
#

if build_lua

# We use two different methods for building liblua.a, depending on whether
# we're using a separate build directory or not, and whether the Lua source
# directory is a subdirectory (and thus is "ours"):
#
# + If building in the source dir, and the Lua source directory is a simple
#   subdirectory of the top-level source dir, we build liblua.a in the Lua
#   source directory and copy it to our top-level directory.  This avoids
#   most assumptions about how make works.
#
# + If using a separate build dir, or if the Lua source directory is
#   "somewhere else", we build liblua.a in the current directory, using the
#   Makefile from the Lua source directory, and VPATH to tell make where to
#   find the source files. This requires that make support at using VPATH
#   to find source files, which seems a reasonable assumption in this case.
#   It also requires that no lua object files have the same name as any of
#   our object files; that's the case currently (all Lua source/object
#   files begin with "l").
#
if build_lua_in_src_dir

liblua.a:
	(cd $(LUA_SRC); $(MAKE) $(AM_MAKEFLAGS) CC='$(CC)' RANLIB='$(RANLIB)' CFLAGS='$(AM_CFLAGS) $(CFLAGS)' liblua.a)
	cp -p $(LUA_SRC)/liblua.a .
	$(RANLIB) $@
clean-local:
	-rm -rf $(LUA_SRC)/liblua.a
mostlyclean-local:
	-rm -rf $(LUA_SRC)/*.o

else !build_lua_in_src_dir

liblua.a:
	$(MAKE) -f $(LUA_SRC)/Makefile $(AM_MAKEFLAGS) CC='$(CC)' RANLIB='$(RANLIB)' CFLAGS='$(AM_CFLAGS) $(CFLAGS)' VPATH=$(LUA_SRC) liblua.a

endif

endif


###############################################################
#
# "Raw" SWIG interface to snogray, libsnograw.a
#

# If the swig interface generator is available, we can update the C++
# interface when snograw.swg changes; otherwise we need to use
# pregenerated copies.
#
if have_swig

SWIG_FLAGS = $(SWIG_NOMODULEGLOBAL_FLAG)

# automake "silent-rules" hooks for swig.
#
swig_verbose = $(swig_verbose_$(V))
swig_verbose_ = $(swig_verbose_$(AM_DEFAULT_VERBOSITY))
swig_verbose_0 = @echo "  SWIG    " $@;

# Build rules for swig-generated files we use.
#
%_lua_wrap.cc : %.swg
	$(swig_verbose)$(SWIG) $(SWIG_FLAGS) -o $@ -c++ -lua $<

else

# A dummy build rule for swig-generated files that gets used when the
# swig program isn't available; it just prints an warning/error
# message when make tries to regenerate the file (which normally
# shouldn't be necessary with a tarball distribution, as
# swig-generated files are distributed in the tarball).
#
%_wrap.cc : %.swg
	@if test -r "$@"; then						       \
	  echo '** Warning: $< is newer than $@, but cannot run swig interface generator'; \
	else								       \
	  echo '** Error: cannot run swig interface generator to generate $@'; \
	  exit 1;							       \
	fi

endif

libsnograw_a_SOURCES = snograw_lua_wrap.cc

# We include some other SWIG files from snograw.swg, so reflect those
# dependencies.
#
snograw_lua_wrap.cc: camera.swg color/color.swg common.swg	\
	image/image.swg light/light.swg load.swg		\
	material/material.swg rdrive.swg render.swg scene.swg	\
	space/space.swg surface/surface.swg texture/texture.swg	\
	util.swg val-table.swg vector.swg


# Compiler options for compiling SWIG-generated Lua interface.
#
# These are the same as the default compiler options, except:
# (1) Optimize for size (SWIG-generated code can be bloated)
# (2) Add "-fno-strict-aliasing", as the generated code uses some
#     dodgy aliasing tricks
# (3) Don't use pedantic checking options (the quality of the
#     generated code is not something we can easily control)
#
libsnograw_a_CXXFLAGS = $(OPT_SIZE) $(DEBUG) $(CXX_STD_FLAGS) -fno-strict-aliasing $(CXX_EXTRA_REQ_FLAGS) $(EXTRA_COMPILE_FLAGS)

# Make automake suck slightly less
libsnograw_a_SHORTNAME = sl

# This makes sure that liblua.a gets built when necessary.
#
if build_lua
libsnograw_a_DEPENDENCIES = liblua.a
endif

## Build a dynamically loadable Lua module, for debugging.
##
snograw_so_LIBS = $(LOAD_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)
snograw_so_DEPS = $(filterout, -l%, $(snograw_so_LIBS)
snograw.so: $(snograw_DEPS)
	$(CXX) -o $@ --shared -Wl,-uluaopen_snograw libsnograw.a \
		$(snograw_so_LIBS)


## arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
