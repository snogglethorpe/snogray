# Automake Makefile template for snogray
#
#  Copyright (C) 2005-2012  Miles Bader <miles@gnu.org>
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


# Lua files loaded directly by the driver.
#
pkgluadir = $(pkgdatadir)/lua
pkglua_DATA = module-setup.lua

# Lua modules (loaded via the Lua 'require' function).
#
pkgluamoduledir = $(pkgluadir)/module

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
pkgluamodulesnograydir = $(pkgluamoduledir)/snogray
pkgluamodulesnogray_DATA = file.lua filename.lua load.lua	\
	lpeg-utils.lua snogray.lua string-fill.lua string.lua	\
	swig.lua table.lua

# Lua loaders for various scene/mesh formats.
#
pkglualoaderdir = $(pkgluadir)/loader
pkglualoader_DATA = load-lua.lua load-nff.lua load-obj.lua	\
	load-pbrt.lua load-stl.lua load-ug.lua


# Internal libraries; not installed
#
noinst_LIBRARIES = libsnogcli.a libsnogcolor.a libsnogimage.a		\
	libsnogimagecli.a libsnoglight.a libsnogload.a			\
	libsnogloaders.a libsnoglua.a libsnogmat.a libsnograw.a		\
	libsnogrdrive.a libsnogrender.a libsnogscene.a			\
	libsnogscenecli.a libsnogspace.a libsnogsurf.a libsnogtex.a	\
	libsnogutil.a libluaexts.a


# Various files to include in distribution not covered by automatic rules
#
EXTRA_DIST =							\
	$(pkglua_DATA)						\
	$(pkgluamodulesnogray_DATA) $(pkglualoader_DATA)	\
	README README.building README.issues README.usage	\
	README.utils						\
	snograw.swg vector.swg val-table.swg			\
	image.swg surface.swg material.swg light.swg tex.swg	\
	scene.swg camera.swg rdrive.swg space.swg color.swg	\
	load.swg util.swg					\
	update-version.sh					\
	autogen.sh						\
	lpeg.html						\
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

# We only clean up "snograw_lua_wrap.cc" and "swigluarun.h" if it's
# possible to regenerate them.
#
if have_swig
CLEANFILES += snograw_lua_wrap.cc swigluarun.h
endif


################################################################
#
# General compilation flags.  "OPT" and "DEBUG" intentionally have short
# names so that it's easy for a user to override them from the command
# line.  CXX_OPT_FLAGS, CXX_OPT_SIZE_FLAGS, CXX_MACH_FLAGS,
# CXX_EXTRA_OPT_FLAGS, and EXTRA_COMPILE_FLAGS are set by configure.
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
AM_CFLAGS = $(OPT) $(DEBUG) $(EXTRA_COMPILE_FLAGS)
AM_CXXFLAGS = $(OPT) $(DEBUG) $(CXX_STD_FLAGS) $(CXX_CHECK_FLAGS) $(EXTRA_COMPILE_FLAGS)

# Versions of AM_CFLAGS and AM_CXXFLAGS for situations where we want to
# optimize for size.
#
AM_CFLAGS_SIZE = $(OPT_SIZE) $(DEBUG) $(EXTRA_COMPILE_FLAGS)
AM_CXXFLAGS_SIZE = $(OPT_SIZE) $(DEBUG) $(CXX_STD_FLAGS) $(CXX_CHECK_FLAGS) $(EXTRA_COMPILE_FLAGS)

# These options are required for correctness, and _must_ come _after_
# any optimization options (gcc isn't very bright about parsing
# options:  later options always override earlier ones, even if the
# earlier one was more specific).
#
AM_CFLAGS += $(CXX_EXTRA_REQ_FLAGS)
AM_CXXFLAGS += $(CXX_EXTRA_REQ_FLAGS)

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

libsnogrender_a_SOURCES = dir-hist.h dir-hist-dist.h direct-illum.cc	\
	direct-illum.h direct-integ.h filter-volume-integ.h		\
	global-render-state.cc global-render-state.h grid.cc grid.h	\
	hist-2d.h hist-2d-dist.h integ.h intersect.cc intersect.h	\
	isec-cache.h media.cc media.h mis-sample-weight.h path-integ.cc	\
	path-integ.h photon-eval.cc photon-eval.h photon-integ.cc	\
	photon-integ.h photon-shooter.cc photon-shooter.h ray.h		\
	ray-io.cc ray-io.h recursive-integ.cc recursive-integ.h		\
	render-context.cc render-context.h render-params.h		\
	render-stats.cc render-stats.h cone-sample.h disk-sample.h	\
	sample-gen.h sample-set.cc sample-set.h sphere-sample.h		\
	tangent-disk-sample.h surface-integ.h volume-integ.h		\
	zero-surface-integ.h


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

libsnogscene_a_SOURCES = bbox.cc bbox.h bbox-io.cc bbox-io.h bsdf.h	\
	camera.cc camera.h coords.h cyl-xform.cc cyl-xform.h dist.h	\
	frame.h local-xform.cc local-xform.h material-dict.cc		\
	material-dict.h material.cc material.h matrix4.cc matrix4.h	\
	matrix4.tcc medium.h photon.h photon-map.cc photon-map.h	\
	pos.h pos-io.cc pos-io.h primitive.cc primitive.h		\
	quadratic-roots.h scene.cc scene.h sphere-isec.h		\
	spherical-coords.h surface.cc surface.h surface-group.cc	\
	surface-group.h surface-light.cc surface-light.h tex.h		\
	tex-coords.h tripar-isec.h triv-space.h tuple3.h uv.h		\
	uv-io.cc uv-io.h vec.h vec-io.cc vec-io.h xform.h		\
	xform-base.h xform-io.cc xform-io.h


################################################################
#
# Snogray acceleration-structure library, libsnogspace.a
#

libsnogspace_a_SOURCES = octree.cc octree.h space.cc space.h		\
	space-builder.h


################################################################
#
# Snogray texture library, libsnogtex.a
#

libsnogtex_a_SOURCES = arith-tex.cc arith-tex.h arith-tex.tcc		\
	check-tex.h cmp-tex.cc cmp-tex.h cmp-tex.tcc coord-tex.h	\
	cubemap.cc cubemap.h envmap.h grey-tex.h intens-tex.h	\
	interp-tex.h matrix-linterp.h matrix-tex.cc matrix-tex.h	\
	matrix-tex.tcc misc-map-tex.h perlin.cc perlin.h perlin-tex.h	\
	perturb-tex.h rescale-tex.h spheremap.cc spheremap.h worley.cc	\
	worley.h worley-tex.h xform-tex.h


################################################################
#
# Snogray command-line-interface library, libsnogcli.a
#

libsnogcli_a_SOURCES = cmdlineparser.cc cmdlineparser.h		\
	tty-progress.cc tty-progress.h version.cc version.h


################################################################
#
# Snogray light library, libsnoglight.a
#

libsnoglight_a_SOURCES = envmap-light.cc envmap-light.h far-light.cc	\
	far-light.h image-sum.h light-map.h light.h point-light.cc	\
	point-light.h sphere-light.cc sphere-light.h


################################################################
#
# Snogray material library, libsnogmat.a
#

libsnogmat_a_SOURCES = cook-torrance.cc cook-torrance.h cos-dist.h	\
	fresnel.h glass.cc glass.h glow.cc glow.h lambert.cc		\
	lambert.h mirror.cc mirror.h norm-glow.cc norm-glow.h		\
	phong-dist.h phong.cc phong.h thin-glass.cc thin-glass.h	\
	stencil.cc stencil.h ward-dist.h


################################################################
#
# Snogray surface library, libsnogsurf.a
#

libsnogsurf_a_SOURCES = cylinder.cc cylinder.h ellipse.cc ellipse.h	\
	instance.cc instance.h local-primitive.h local-surface.h	\
	mesh.cc mesh.h model.cc model.h sphere.cc sphere.h sphere2.cc	\
	sphere2.h tessel.cc tessel.h tessel-param.cc tessel-param.h	\
	tessel-sinc.cc tessel-sinc.h tessel-sphere.cc tessel-sphere.h	\
	tessel-torus.cc tessel-torus.h tripar.cc tripar.h


################################################################
#
# Snogray scene-loading library, libsnogload.a
#

libsnogload_a_SOURCES = camera-cmds.cc camera-cmds.h load-lua.cc load-lua.h

# Library dependencies of libsnogload.a
#
libsnogload_LIBS = 


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

libsnoglua_a_SOURCES = lua-funs.cc lua-funs.h lua-setup.cc lua-setup.h

# Library dependencies of libsnoglua.a
#
libsnoglua_LIBS = libsnograw.a libluaexts.a $(liblua_LIBS)

load-lua.o : swigluarun.h


################################################################
#
# Library of statically-linked lua extension modules.  These could
# also be dynamically loaded, but that needs more infrastructure work.
#

libluaexts_a_SOURCES = lpeg.c

libluaexts_a_CFLAGS = $(AM_CFLAGS) -DNDEBUG

# Make automake suck slightly less
libluaexts_a_SHORTNAME = le


################################################################
#
# Snogray image I/O library, libsnogimage.a
#

libsnogimage_a_SOURCES = image.h image-box-filt.h image-byte-vec.cc	\
	image-byte-vec.h image-dispatch.cc image-filter-conv.h		\
	image-filter.cc image-filter.h image-gauss-filt.h image-input.h	\
	image-io.cc image-io.h image-mitchell-filt.h			\
	image-sampled-output.cc image-sampled-output.h			\
	image-scaled-output.cc image-scaled-output.h image-pfm.cc	\
	image-pfm.h image-rgbe.cc image-rgbe.h image-tga.cc image-tga.h	\
	image-triangle-filt.h tuple-adaptor.h tuple-matrix.cc		\
	tuple-matrix.h tuple-matrix.tcc

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
# Snogray image command-line handling library, libsnogimagecli.a
#

libsnogimagecli_a_SOURCES = image-input-cmdline.h			\
	image-sampled-output-cmdline.cc image-sampled-output-cmdline.h	\
	image-scaled-output-cmdline.h


################################################################
#
# Snogray color library, libsnogcolor.a
#

libsnogcolor_a_SOURCES = color.cc color.h color-io.cc color-io.h	\
	color-math.h tint.h tint-io.cc tint-io.h


################################################################
#
# Snogray general utility library, libsnogutil.a
#

libsnogutil_a_SOURCES = compiler.h cond-var.h excepts.h file-funs.cc	\
	file-funs.h float-excepts-guard.h freelist.cc freelist.h	\
	funptr-cast.h globals.cc globals.h grab.h interp.h llist.h	\
	least-squares-fit.h matrix.h matrix.tcc matrix-funs.h		\
	matrix-funs.tcc matrix-io.h mempool.cc mempool.h mutex.h	\
	nice-io.cc nice-io.h num-cores.cc num-cores.h pool.h		\
	progress.h radical-inverse.h random.h random-boost.h		\
	random-std.h random-rand.h random-tr1.h ref.h rusage.h		\
	snogassert.cc snogassert.h snogmath.h snogpaths.cc		\
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
snogpaths_verbose_0 = @echo "  PATHS " $@;


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
	libsnogtex.a libsnogspace.a libsnogscene.a
LOAD_LIBS = libsnogload.a $(libsnogload_LIBS)	\
	libsnoglua.a $(libsnoglua_LIBS)	\
	libsnogloaders.a $(libsnogloaders_LIBS)
IMAGE_LIBS = libsnogimagecli.a libsnogimage.a $(libsnogimage_LIBS)	\
	libsnogcolor.a
MISC_LIBS = libsnogcli.a libsnogutil.a


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc recover-image.cc recover-image.h
snogray_LDADD = libsnogscenecli.a libsnogrdrive.a $(LOAD_LIBS)	\
	$(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = $(LOAD_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)

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
swig_verbose_0 = @echo "  SWIG  " $@;

# Build rules for swig-generated files we use.
#
%_lua_wrap.cc : %.swg
	$(swig_verbose)$(SWIG) $(SWIG_FLAGS) -o $@ -c++ -lua $<
swigluarun.h:
	$(swig_verbose)$(SWIG) $(SWIG_FLAGS) -lua -c++ -external-runtime $@

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

libsnograw_a_SOURCES = snograw_lua_wrap.cc swigluarun.h

# We include some other SWIG files from snograw.swg, so reflect those
# dependencies.
#
snograw_lua_wrap.cc: vector.swg val-table.swg image.swg surface.swg	\
	material.swg light.swg tex.swg scene.swg camera.swg color.swg	\
	rdrive.swg space.swg load.swg util.swg

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
