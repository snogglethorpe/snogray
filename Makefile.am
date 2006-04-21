# Automake Makefile template for snogray
#
#  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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

# Internal libraries; not installed
#
noinst_LIBRARIES = libsnogrender.a libsnogimage.a libsnogmisc.a


################################################################
#
# General compilation flags
#

OPT = -O5 -fomit-frame-pointer -ffast-math $(MACHINE_OPT)
DEBUG = -g -Wall
#PG = -pg
#MUDFLAP = -fmudflap

# Even though CFLAGS and CXXFLAGS are the "user flags", automake uses
# them to define optimization flags, so we must do so as well.  The user
# can also use them to override our optimization choice.
#
CFLAGS = $(OPT) $(DEBUG)
CXXFLAGS = $(CFLAGS)

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
	$(libnetpbm_CFLAGS) $(LIB3DS_CFLAGS)


################################################################
#
# Per-host configuration
#

HOST_OPT_dhapc248.dev.necel.com = $(ARCH_OPT_pentium4)
ARCH_OPT_pentium3 = -march=pentium3 -mfpmath=sse -msse
ARCH_OPT_pentium4 = -march=pentium4 -mfpmath=sse -msse2
ARCH_OPT_i686 = $(ARCH_OPT_pentium3)

HOST := $(shell hostname)
ARCH := $(shell uname -m)
HOST_OPT_$(HOST) ?= $(ARCH_OPT_$(ARCH))
MACHINE_OPT = $(HOST_OPT_$(HOST))


################################################################
#
# Snogray rendering library, libsnogrender.a
#

libsnogrender_a_SOURCES = bbox.h brdf.cc brdf.h camera.cc camera.h	\
	cook-torrance.cc cook-torrance.h coords.h cubetex.cc cubetex.h	\
	excepts.h far-light.cc far-light.h fresnel.h glass.cc glass.h	\
	global-tstate.h glow.cc glow.h intersect.cc intersect.h		\
	lambert.cc lambert.h light.cc light.h lsamples.cc lsamples.h	\
	material.cc material.h matrix4.h medium.h mesh.cc mesh.h	\
	mesh-load.cc mirror.cc mirror.h octree.cc octree.h phong.cc	\
	phong.h plastic.cc plastic.h point-light.cc point-light.h pos.h	\
	progress.cc progress.h rand.h ray.cc ray.h rect-light.cc	\
	rect-light.h sample-ray.h scene.cc scene.h scene-def.cc		\
	scene-def.h scene-load.cc scene-load-aff.cc space.cc space.h	\
	sphere.cc sphere.h surface.cc surface.h tessel.cc tessel.h	\
	tessel-param.cc tessel-param.h test-scenes.cc test-scenes.h	\
	texture2.cc texture2.h timeval.cc timeval.h trace.cc trace.h	\
	tripar.cc tripar.h tripar-isec.h tuple3.h vec.h wire-frame.h	\
	xform.h

# Library dependencies of libsnogrender.a
#
RENDER_LIBS =

if have_lib3ds
  libsnogrender_a_SOURCES += load-3ds.cc load-3ds.h
  RENDER_LIBS += $(LIB3DS_LIBS)
endif


################################################################
#
# Snogray image I/O library, libsnogimage.a
#

libsnogimage_a_SOURCES = image.cc image.h image-aa.cc image-byte-vec.cc	\
	image-byte-vec.h image-cmdline.cc image-cmdline.h		\
	image-dispatch.cc image-io.cc image-io.h image-pfm.cc		\
	image-pfm.h image-rgbe.cc image-rgbe.h

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

libsnogmisc_a_SOURCES = cmdlineparser.cc cmdlineparser.h color.cc color.h \
	freelist.cc freelist.h llist.h rusage.h string-funs.cc string-funs.h


################################################################
#
# Extra library configuration
#

LIBS += $(MUDFLAP:-f%=-l%)


###############################################################
#
# Programs
#

snogray_SOURCES = snogray.cc trace-stats.cc trace-stats.h
snogray_LDADD = libsnogrender.a libsnogimage.a libsnogmisc.a \
	$(RENDER_LIBS) $(IMAGE_LIBS) $(LIBS)

snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = libsnogrender.a libsnogimage.a libsnogmisc.a \
	$(RENDER_LIBS) $(IMAGE_LIBS) $(LIBS)

snogcvt_SOURCES = snogcvt.cc
snogcvt_LDADD = libsnogimage.a libsnogmisc.a $(IMAGE_LIBS) $(LIBS)

snogdiff_SOURCES = snogdiff.cc
snogdiff_LDADD = libsnogimage.a libsnogmisc.a $(IMAGE_LIBS) $(LIBS)


## arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
