# Automake Makefile template for snogray
#
#  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
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
bin_PROGRAMS = snogray snogcvt snogdiff
noinst_PROGRAMS =


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


# Internal libraries; not installed
#
noinst_LIBRARIES = libsnogcli.a libsnogimage.a libsnoglight.a		\
	libsnogload.a libsnogload2.a libsnogmat.a libsnogrdrive.a	\
	libsnogrender.a libsnogscene.a libsnogsurf.a libsnogtex.a	\
	libsnogutil.a

if use_lua
  noinst_LIBRARIES += libsnograw.a libluaexts.a
endif


# Various files to include in distribution not covered by automatic rules
#
EXTRA_DIST =							\
	snogray.lua						\
	load-obj.lua load-ug.lua load-stl.lua load-nff.lua	\
	snograw.swg						\
	autogen.sh						\
	lpeg.html						\
	stats.txt spd.txt

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
CLEANFILES = liblua.a snograw.so

# We only clean up "snograw_lua_wrap.cc" and "swigluarun.h" if it's
# possible to regenerate them.
#
if have_swig
MAINTAINERCLEANFILES = snograw_lua_wrap.cc swigluarun.h
endif


################################################################
#
# General compilation flags.  "OPT" and "DEBUG" intentionally have short
# names so that it's easy for a user to override them from the command
# line.  CXX_OPT_FLAGS, CXX_OPT_SIZE_FLAGS, CXX_MACH_FLAGS and
# CXX_EXTRA_OPT_FLAGS are set by configure.
#
OPT = $(CXX_OPT_FLAGS) $(CXX_EXTRA_OPT_FLAGS) $(CXX_MACH_FLAGS)
OPT_SIZE = $(CXX_OPT_SIZE_FLAGS) $(CXX_EXTRA_OPT_FLAGS) $(CXX_MACH_FLAGS)
DEBUG = -g

# Override automake's default values for CFLAGS and CXXFLAGS.  If the
# user specifies his own values on the command line, his values will
# override these (make ignores assignments to variables with a value
# specified by the user on the make command line).
#
# Generally the user shouldn't use CFLAGS or CXXFLAGS though -- rather
#  he should use the OPT and DEBUG variables, which allow similar
#  control in a more controllable manner.
#
CFLAGS =
CXXFLAGS =

# Set the default compiler options from OPT and DEBUG.  If the user
# wants to change the optimization or debug settings, he should set one
# of those on the make command line.
#
AM_CFLAGS = $(OPT) $(DEBUG) $(MISC_COMPILE_FLAGS)
AM_CXXFLAGS = $(OPT) $(DEBUG) $(CXX_CHECK_FLAGS) $(MISC_COMPILE_FLAGS)

# These options are required for correctness, and _must_ come _after_
# any optimization options (gcc isn't very bright about parsing options:
# later options always override earlier ones, even if the earlier one
# was more specific).  Since automake always passes CXXFLAGS last, and even 
#
CXXFLAGS += $(CXX_EXTRA_REQ_FLAGS)

# Add compiler flags (nominally "cflags", but in practice always
# preprocessor options) needed by libraries we use.
#
# It would be nice to only use these flags when compiling the actual
# object file which needs them -- however automake has very poor support
# for specifying non-global compiler options (even where it can be done,
# automake's implementation has annoying side-effects).
#
AM_CPPFLAGS = $(libpng_CFLAGS) $(libexr_CFLAGS) $(libjpeg_CFLAGS)	\
	$(libnetpbm_CFLAGS) $(LIB3DS_CFLAGS) $(liblua_CFLAGS)


################################################################
#
# Snogray core rendering library, libsnogrender.a
#

libsnogrender_a_SOURCES = direct-illum.cc direct-illum.h		\
	global-tstate.cc global-tstate.h grid-iter.h grid.cc grid.h	\
	illum.h illum-mgr.cc illum-mgr.h illum-sample.h intersect.cc	\
	intersect.h isec-cache.h mis-illum.cc mis-illum.h ray.cc ray.h	\
	recurs-illum.cc recurs-illum.h sample-disk.h sample2-gen.h	\
	shadow-ray.h trace-params.h trace-stats.cc trace-stats.h	\
	trace.cc trace.h


################################################################
#
# Snogray rendering driver library, libsnogrdrive.a
#

libsnogrdrive_a_SOURCES = progress.cc progress.h render-cmdline.h	\
	render.cc render.h renderer.cc renderer.h wire-frame.h


################################################################
#
# Snogray scene-representation library, libsnogscene.a
#

libsnogscene_a_SOURCES = bbox.cc bbox.h brdf.h camera.cc camera.h	\
	coords.h dist.h frame.h material-dict.cc material-dict.h	\
	material.cc material.h matrix4.h medium.h octree.cc octree.h	\
	pos.h quadratic-roots.h scene.cc scene.h space.cc space.h	\
	space-builder.h sphere-isec.h surface.cc surface.h tex.h	\
	tex-coords.h tripar-isec.h tuple3.h uv.h vec.h xform.h		\
	xform-base.h


################################################################
#
# Snogray texture library, libsnogtex.a
#

libsnogtex_a_SOURCES = arith-tex.cc arith-tex.h arith-tex.tcc		\
	check-tex.h cubemap.cc cubemap.h envmap.cc envmap.h grey-tex.h	\
	intens-tex.h matrix-linterp.h matrix-tex.cc matrix-tex.h	\
	matrix-tex.tcc misc-tex.h misc-map-tex.h perlin.cc perlin.h	\
	perlin-tex.h spheremap.cc spheremap.h xform-tex.h


################################################################
#
# Snogray command-line-interface library, libsnogcli.a
#

libsnogcli_a_SOURCES = recover.cc recover.h


################################################################
#
# Snogray light library, libsnoglight.a
#

libsnoglight_a_SOURCES = envmap-light.cc envmap-light.h far-light.cc	\
	far-light.h image-sum.h light-map.h light.h lmap-analyzer.cc	\
	lmap-analyzer.h point-light.cc point-light.h rect-light.cc	\
	rect-light.h sphere-light.cc sphere-light.h struct-light.cc	\
	struct-light.h


################################################################
#
# Snogray material library, libsnogmat.a
#

libsnogmat_a_SOURCES = cook-torrance.cc cook-torrance.h cos-dist.h	\
	fresnel.h glass.cc glass.h glow.cc glow.h lambert.cc lambert.h	\
	mirror.cc mirror.h norm-glow.cc norm-glow.h phong-dist.h	\
	phong.cc phong.h plastic.cc plastic.h ward-dist.h


################################################################
#
# Snogray surface library, libsnogsurf.a
#

libsnogsurf_a_SOURCES = cylinder.cc cylinder.h ellipse.cc ellipse.h	\
	instance.cc instance.h local-surface.cc local-surface.h mesh.cc	\
	mesh.h sphere.cc sphere.h sphere2.cc sphere2.h subspace.cc	\
	subspace.h surface-group.cc surface-group.h tessel-param.cc	\
	tessel-param.h tessel.cc tessel.h tripar.cc tripar.h


################################################################
#
# Snogray scene-loading library, libsnogload.a
#

libsnogload_a_SOURCES = camera-cmds.cc camera-cmds.h scene-def.cc	\
	scene-def.h scene-load.cc

# Library dependencies of libsnogload.a
#
libsnogload_LIBS =

if use_lua
  libsnogload_a_SOURCES += load-lua.cc load-lua.h
  libsnogload_LIBS += libsnograw.a libluaexts.a $(liblua_LIBS)

load-lua.o : swigluarun.h
endif


################################################################
#
# Library of statically-linked lua extensions.  These could also be
# dynamically loaded, but that needs more infrastructure work.
#

libluaexts_a_SOURCES = lpeg.c

libluaexts_a_CFLAGS = $(AM_CFLAGS) -DNDEBUG

# Make automake suck slightly less
libluaexts_a_SHORTNAME = le


################################################################
#
# Snogray scene-loading library #2 (we need two because of link-order
# issues), libsnogload2.a
#

libsnogload2_a_SOURCES = mesh-load.cc
libsnogload2_a_SOURCES += load-msh.cc load-msh.h
libsnogload2_a_SOURCES += load-ply.cc load-ply.h rply.c rply.h

# Library dependencies of libsnogload2.a
#
libsnogload2_LIBS =

if have_lib3ds
  libsnogload2_a_SOURCES += load-3ds.cc load-3ds.h
  libsnogload2_LIBS += $(LIB3DS_LIBS)
endif


################################################################
#
# Snogray image I/O library, libsnogimage.a
#

libsnogimage_a_SOURCES = box-filt.h filter.cc filter.h filter-conv.h	 \
	gauss-filt.h image.h image-byte-vec.cc image-byte-vec.h		 \
	image-cmdline.h tuple-matrix.cc tuple-matrix.h image-dispatch.cc \
	image-dtors.cc image-input.h image-io.cc image-io.h		 \
	image-output.cc image-output.h image-pfm.cc image-pfm.h		 \
	image-rgbe.cc image-rgbe.h mitchell-filt.h

# Library dependencies of libsnogimage.a
#
libsnogimage_LIBS =

if have_libpng
  libsnogimage_a_SOURCES += image-png.cc image-png.h
  libsnogimage_LIBS += $(libpng_LIBS)
endif

if have_libexr
  libsnogimage_a_SOURCES += image-exr.cc image-exr.h
  libsnogimage_LIBS += $(libexr_LIBS)
endif

if have_libjpeg
  libsnogimage_a_SOURCES += image-jpeg.cc image-jpeg.h
  libsnogimage_LIBS += $(libjpeg_LIBS)
endif

if have_libnetpbm
  libsnogimage_a_SOURCES += image-ppm.cc image-ppm.h
  libsnogimage_LIBS += $(libnetpbm_LIBS)
endif


################################################################
#
# Snogray general utility library, libsnogutil.a
#

libsnogutil_a_SOURCES = cmdlineparser.cc cmdlineparser.h color.cc	\
	color.h excepts.h file-funs.cc file-funs.h freelist.cc		\
	freelist.h globals.cc globals.h grab.h interp.h llist.h		\
	mempool.cc mempool.h pool.h rand.h ref.h rusage.h snogmath.h	\
	string-funs.cc string-funs.h timeval.cc timeval.h val-table.cc	\
	val-table.h


################################################################
#
# Extra library configuration
#

LIBS += $(MUDFLAP:-f%=-l%)


################################################################
#
# Groups of associated libraries
#
CORE_LIBS = libsnogmat.a libsnogsurf.a libsnoglight.a libsnogrender.a	\
	libsnogtex.a libsnogscene.a
LOAD_LIBS = libsnogload.a $(libsnogload_LIBS) libsnogload2.a	\
	$(libsnogload2_LIBS)
IMAGE_LIBS = libsnogimage.a $(libsnogimage_LIBS)
MISC_LIBS = libsnogutil.a $(LIBS)


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc
snogray_LDADD = libsnogcli.a libsnogrdrive.a $(LOAD_LIBS) $(CORE_LIBS)	\
	$(IMAGE_LIBS) $(MISC_LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = libsnogcli.a $(LOAD_LIBS) $(CORE_LIBS) $(IMAGE_LIBS)	\
	$(MISC_LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = $(IMAGE_LIBS) $(MISC_LIBS)

hemint_SOURCES = hemint.cc
hemint_LDADD = libsnogutil.a


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
	(cd $(LUA_SRC); $(MAKE) $(AM_MAKEFLAGS) liblua.a)
	cp -p $(LUA_SRC)/liblua.a .
clean-local:
	-rm -rf $(LUA_SRC)/liblua.a
mostlyclean-local:
	-rm -rf $(LUA_SRC)/*.o

else !build_lua_in_src_dir

liblua.a:
	$(MAKE) -f $(LUA_SRC)/Makefile $(AM_MAKEFLAGS) VPATH=$(LUA_SRC) liblua.a

endif

endif


###############################################################
#
# "Raw" swig interface to snogray.
#

# If the swig interface generator is available, we can update the C++
# interface when snograw.swg changes; otherwise we need to use
# pregenerated copies.
#
if have_swig

%_lua_wrap.cc : %.swg
	$(SWIG) -o $@ -c++ -lua $<
swigluarun.h:
	$(SWIG) -lua -c++ -external-runtime $@

else

%_wrap.cc : %.swg
	@if test -r "$@"; then						       \
	  echo '** Warning: $< is newer than $@, but cannot run swig interface generator'; \
	else								       \
	  echo '** Error: cannot run swig interface generator to generate $@'; \
	  exit 1;							       \
	fi

endif

libsnograw_a_SOURCES = snograw_lua_wrap.cc swigluarun.h

# Note that we don't copy AM_CFLAGS -- that's because we want to use a
# different optimization setting, $(OPT_SIZE) instead of $(OPT)
#
libsnograw_a_CXXFLAGS = $(OPT_SIZE) $(DEBUG) $(CXX_CHECK_FLAGS)	\
			-fno-strict-aliasing -Wno-unused

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
