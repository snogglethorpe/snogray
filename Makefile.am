# Automake Makefile template for snogray
#
#  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
#
# This file is subject to the terms and conditions of the GNU General
# Public License.  See the file COPYING in the main directory of this
# archive for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


# Targets
#
bin_PROGRAMS = snogray snogcvt snogdiff snogsamp
noinst_PROGRAMS = hemint

# Internal libraries; not installed
#
noinst_LIBRARIES = libsnogrender.a libsnogimage.a libsnogmisc.a

if lua_scene_def
  noinst_LIBRARIES += libsnograw.a
endif


# Various files to include in distribution not covered by automatic rules
#
EXTRA_DIST = autogen.sh					\
	board1.msh board2.msh board3.msh bunny500.msh	\
	stats.txt spd.txt


################################################################
#
# General compilation flags.  "OPT" and "DEBUG" intentionally have short
# names so that it's easy for a user to override them from the command
# line.  CXX_OPT_FLAGS, CXX_MACH_FLAGS and CXX_EXTRA_OPT_FLAGS are set
# by automake/autoconf.
#
OPT = $(CXX_OPT_FLAGS) $(CXX_EXTRA_OPT_FLAGS) $(CXX_MACH_FLAGS)
DEBUG = -g

#PG = -pg
#MUDFLAP = -fmudflap

# Even though CFLAGS and CXXFLAGS are the "user flags", automake uses
# them to define optimization flags, so we must do so as well.  The user
# can also use them to override our optimization choice.  CXX_CHECK_FLAGS
# is set by automake/autoconf.
#
CFLAGS = $(OPT) $(DEBUG)
CXXFLAGS = $(CFLAGS) $(CXX_CHECK_FLAGS)

# Other flags we pass via AM_CXXFLAGS
#
AM_CXXFLAGS = $(PG) $(MUDFLAP) $(CXX_EXTRA_REQ_FLAGS)
AM_LDFLAGS = $(PG) $(MUDFLAP)

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
# Snogray rendering library, libsnogrender.a
#

libsnogrender_a_SOURCES = bbox.h brdf.h camera.cc camera.h		 \
	camera-cmds.cc camera-cmds.h cook-torrance.cc cook-torrance.h	 \
	coords.h cos-dist.h cubemap.cc cubemap.h dist.h envmap.cc	 \
	envmap.h envmap-light.cc envmap-light.h excepts.h far-light.cc	 \
	far-light.h fresnel.h glass.cc glass.h globals.cc globals.h	 \
	global-tstate.cc global-tstate.h glow.cc glow.h grid.cc grid.h	 \
	grid-iter.h illum.h illum-sample.h image-sum.h intersect.cc	 \
	intersect.h lambert.cc lambert.h light.h light-map.h		 \
	lmap-analyzer.cc lmap-analyzer.h material.cc material.h		 \
	material-map.cc material-map.h matrix4.h matrix-tex2.cc		 \
	matrix-tex2.h matrix-tex2.tcc medium.h mesh.cc mesh.h		 \
	mesh-load.cc mirror.cc mirror.h mis-illum.cc mis-illum.h	 \
	octree.cc octree.h phong.cc phong.h phong-dist.h plastic.cc	 \
	plastic.h point-light.cc point-light.h pos.h progress.cc	 \
	progress.h rand.h ray.cc ray.h rect-light.cc rect-light.h	 \
	render.cc render.h render-cmdline.h renderer.cc renderer.h	 \
	sample-disk.h sample-illum.cc sample-illum.h sample2-gen.h	 \
	scene.cc scene.h scene-def.cc scene-def.h scene-load-aff.cc	 \
	scene-load.cc snogmath.h space.cc space.h sphere.cc sphere.h	 \
	sphere-isec.h sphere-light.cc sphere-light.h spheremap.cc	 \
	spheremap.h struct-light.cc struct-light.h surface.cc surface.h	 \
	tessel.cc tessel.h tessel-param.cc tessel-param.h test-scenes.cc \
	test-scenes.h tex2.h timeval.cc timeval.h trace.cc trace.h	 \
	trace-params.h trace-stats.cc trace-stats.h tripar.cc tripar.h	 \
	tripar-isec.h tuple3.h uv.h vec.h ward-dist.h wire-frame.h	 \
	xform.h

# Library dependencies of libsnogrender.a
#
RENDER_LIBS =

libsnogrender_a_SOURCES += load-ply.cc load-ply.h rply.c rply.h

if have_lib3ds
  libsnogrender_a_SOURCES += load-3ds.cc load-3ds.h
  RENDER_LIBS += $(LIB3DS_LIBS)
endif

if lua_scene_def
  libsnogrender_a_SOURCES += load-lua.cc load-lua.h
  RENDER_LIBS += libsnograw.a $(liblua_LIBS)

load-lua.o : swigluarun.h
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
IMAGE_LIBS =

if have_libpng
  libsnogimage_a_SOURCES += image-png.cc image-png.h
  IMAGE_LIBS += $(libpng_LIBS)
endif

if have_libexr
  libsnogimage_a_SOURCES += image-exr.cc image-exr.h
  IMAGE_LIBS += $(libexr_LIBS)
endif

if have_libjpeg
  libsnogimage_a_SOURCES += image-jpeg.cc image-jpeg.h
  IMAGE_LIBS += $(libjpeg_LIBS)
endif

if have_libnetpbm
  libsnogimage_a_SOURCES += image-ppm.cc image-ppm.h
  IMAGE_LIBS += $(libnetpbm_LIBS)
endif


################################################################
#
# Snogray utility library, libsnogmisc.a
#

libsnogmisc_a_SOURCES = cmdlineparser.cc cmdlineparser.h color.cc	 \
	color.h file-funs.cc file-funs.h freelist.cc freelist.h llist.h	 \
	recover.cc recover.h ref.h rusage.h string-funs.cc string-funs.h \
	val-table.cc val-table.h


################################################################
#
# Extra library configuration
#

LIBS += $(MUDFLAP:-f%=-l%)


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc
snogray_LDADD = libsnogrender.a libsnogimage.a libsnogmisc.a \
	$(RENDER_LIBS) $(IMAGE_LIBS) $(LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = libsnogrender.a libsnogimage.a libsnogmisc.a \
	$(RENDER_LIBS) $(IMAGE_LIBS) $(LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = libsnogimage.a libsnogmisc.a $(IMAGE_LIBS) $(LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = libsnogimage.a libsnogmisc.a $(IMAGE_LIBS) $(LIBS)

hemint_SOURCES = hemint.cc
hemint_LDADD = libsnogmisc.a


###############################################################
#
# "Raw" swig interface to snogray.
#

%_wrap.cc : %.i
	$(SWIG) -o $@ -c++ -lua $<

swigluarun.h:
	$(SWIG) -lua -c++ -external-runtime $@

libsnograw_a_SOURCES = snograw_wrap.cc
libsnograw_a_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS)

# Make automake suck slightly less
libsnograw_a_SHORTNAME = sl

## XXX
snograw.so : libsnogrender.a libsnogmisc.a
	$(CXX) -o $@ --shared -Wl,-uluaopen_snograw libsnograw.a $^ $(RENDER_LIBS)


## arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
