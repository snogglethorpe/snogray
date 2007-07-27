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
noinst_LIBRARIES = libsnogcli.a libsnogimage.a libsnoglight.a		\
	libsnogload.a libsnogload2.a libsnogmat.a libsnogrdrive.a	\
	libsnogrender.a libsnogscene.a libsnogsurf.a libsnogutil.a

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

# These options are required for correctness, and _must_ come _after_
# any optimization options (gcc isn't very bright about parsing options:
# later options always override earlier ones, even if the earlier one
# was more specific).  Since we are forced to used CXXFLAGS to pass
# optimizations options by automake, we munge it here... Ugh!
#
CXXFLAGS += $(CXX_EXTRA_REQ_FLAGS)

# Other flags we pass via AM_CXXFLAGS
#
AM_CXXFLAGS = $(PG) $(MUDFLAP)
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
# Snogray core rendering library, libsnogrender.a
#

libsnogrender_a_SOURCES = global-tstate.cc global-tstate.h grid-iter.h	\
	grid.cc grid.h illum-sample.h illum.h intersect.cc intersect.h	\
	isec-cache.h mis-illum.cc mis-illum.h ray.cc ray.h		\
	sample-disk.h sample-illum.cc sample-illum.h sample2-gen.h	\
	trace-params.h trace-stats.cc trace-stats.h trace.cc trace.h


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

libsnogscene_a_SOURCES = bbox.h brdf.h camera.cc camera.h coords.h	\
	cubemap.cc cubemap.h dist.h envmap.cc envmap.h material-map.cc	\
	material-map.h material.cc material.h matrix-tex2.cc		\
	matrix-tex2.h matrix-tex2.tcc matrix4.h medium.h octree.cc	\
	octree.h pos.h scene.cc scene.h space.cc space.h sphere-isec.h	\
	spheremap.cc spheremap.h surface.cc surface.h tex2.h		\
	tripar-isec.h tuple3.h uv.h vec.h xform.h


################################################################
#
# Snogray command-line-interface library, libsnogcli.a
#

libsnogcli_a_SOURCES = recover.cc recover.h test-scenes.cc test-scenes.h


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
	mirror.cc mirror.h phong-dist.h phong.cc phong.h plastic.cc	\
	plastic.h ward-dist.h


################################################################
#
# Snogray surface library, libsnogsurf.a
#

libsnogsurf_a_SOURCES = mesh.cc mesh.h sphere.cc sphere.h		\
	tessel-param.cc tessel-param.h tessel.cc tessel.h tripar.cc	\
	tripar.h


################################################################
#
# Snogray scene-loading library, libsnogload.a
#

libsnogload_a_SOURCES = camera-cmds.cc camera-cmds.h scene-def.cc	\
	scene-def.h scene-load.cc

# Library dependencies of libsnogload.a
#
LOAD_LIBS =

if lua_scene_def
  libsnogload_a_SOURCES += load-lua.cc load-lua.h
  LOAD_LIBS += libsnograw.a $(liblua_LIBS)

load-lua.o : swigluarun.h
endif


################################################################
#
# Snogray scene-loading library #2 (we need two because of link-order
# issues), libsnogload2.a
#

libsnogload2_a_SOURCES = mesh-load.cc scene-load-aff.cc

# Library dependencies of libsnogload2.a
#
LOAD2_LIBS =

libsnogload2_a_SOURCES += load-ply.cc load-ply.h rply.c rply.h

if have_lib3ds
  libsnogload2_a_SOURCES += load-3ds.cc load-3ds.h
  LOAD2_LIBS += $(LIB3DS_LIBS)
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
# Snogray general utility library, libsnogutil.a
#

libsnogutil_a_SOURCES = cmdlineparser.cc cmdlineparser.h color.cc	\
	color.h excepts.h file-funs.cc file-funs.h freelist.cc		\
	freelist.h globals.cc globals.h llist.h mempool.cc mempool.h	\
	rand.h ref.h rusage.h snogmath.h string-funs.cc string-funs.h	\
	timeval.cc timeval.h val-table.cc val-table.h


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
snogray_LDADD = libsnogcli.a libsnogrdrive.a libsnogload.a $(LOAD_LIBS)	\
	libsnogload2.a $(LOAD2_LIBS) libsnogmat.a libsnogsurf.a		\
	libsnoglight.a libsnogscene.a libsnogrender.a libsnogimage.a	\
	libsnogutil.a $(IMAGE_LIBS) $(LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = libsnogcli.a libsnogload.a $(LOAD_LIBS) libsnogload2.a	\
	$(LOAD2_LIBS) libsnogmat.a libsnogsurf.a libsnoglight.a		\
	libsnogscene.a libsnogrender.a libsnogimage.a libsnogutil.a	\
	$(IMAGE_LIBS) $(LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = libsnogimage.a libsnogutil.a $(IMAGE_LIBS) $(LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = libsnogimage.a libsnogutil.a $(IMAGE_LIBS) $(LIBS)

hemint_SOURCES = hemint.cc
hemint_LDADD = libsnogutil.a


###############################################################
#
# "Raw" swig interface to snogray.
#

%_wrap.cc : %.i
	$(SWIG) -o $@ -c++ -lua $<

swigluarun.h:
	$(SWIG) -lua -c++ -external-runtime $@

libsnograw_a_SOURCES = snograw_wrap.cc swigluarun.h
libsnograw_a_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS)

# Make automake suck slightly less
libsnograw_a_SHORTNAME = sl

## XXX
snograw.so : libsnogrender.a libsnogutil.a
	$(CXX) -o $@ --shared -Wl,-uluaopen_snograw libsnograw.a $^ $(RENDER_LIBS)


## arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
