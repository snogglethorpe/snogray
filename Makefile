# Makefile for snogray
#
#  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
#
# This file is subject to the terms and conditions of the GNU General
# Public License.  See the file COPYING in the main directory of this
# archive for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

TARGETS = snogray snogcvt snogdiff snogsamp

all: $(TARGETS)

OPT = -O5 -fomit-frame-pointer
DEBUG = -g -Wall
#PG = -pg
#MUDFLAP = -fmudflap

CFLAGS = $(OPT) $(DEBUG) $(MACHINE_CFLAGS) $(PG) $(MUDFLAP)
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(PG) $(MUDFLAP)

################################################################
##
## Per-host configuration
##

HOST_CFLAGS_dhapc248.dev.necel.com = $(ARCH_CFLAGS_pentium4)
ARCH_CFLAGS_pentium3 = -march=pentium3 -mfpmath=sse -msse
ARCH_CFLAGS_pentium4 = -march=pentium4 -mfpmath=sse -msse2
ARCH_CFLAGS_i686 = $(ARCH_CFLAGS_pentium3)

HOST := $(shell hostname)
ARCH := $(shell uname -m)
HOST_CFLAGS_$(HOST) ?= $(ARCH_CFLAGS_$(ARCH))
MACHINE_CFLAGS = $(HOST_CFLAGS_$(HOST))

################################################################
##
## Library configuration (mainly for image backends currently)
##

LIBPNG_CFLAGS	= $(shell libpng-config --cflags)
LIBPNG_LIBS	= $(shell libpng-config --libs)

LIBEXR_CFLAGS	= $(shell pkg-config OpenEXR --cflags)
LIBEXR_LIBS	= $(shell pkg-config OpenEXR --libs)

LIBJPEG_CFLAGS	=
LIBJPEG_LIBS	= -ljpeg

LIBNETPBM_CFLAGS =
LIBNETPBM_LIBS	= -lnetpbm

IMAGE_LIBS =	$(LIBPNG_LIBS) $(LIBEXR_LIBS) $(LIBJPEG_LIBS) $(LIBNETPBM_LIBS)

LIBS =		$(IMAGE_LIBS) $(MUDFLAP:-f%=-l%)

################################################################
##
## Grotty internal details of generating compiler options
##

# Tell gcc to generate dependency files
#
DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

# If compiling with -pg option, we can't use -fomit-frame-pointer, so
# filter it out.
#
_CFLAGS_FILT = $(if $(filter -pg,$(CFLAGS)),$(filter-out -fomit-frame-pointer,$(CFLAGS)),$(CFLAGS))
_CXXFLAGS_FILT = $(if $(filter -pg,$(CXXFLAGS)),$(filter-out -fomit-frame-pointer,$(CXXFLAGS)),$(CXXFLAGS))

# _CFLAGS and _CXXFLAGS are what the actual build rules use
#
_CFLAGS = $(_CFLAGS_FILT) $(DEP_CFLAGS)
_CXXFLAGS = $(_CXXFLAGS_FILT) $(DEP_CFLAGS)

################################################################
##
## Common sources between snogray and utility programs
##

IMAGE_SRCS = image.cc image-aa.cc image-byte-vec.cc image-cmdline.cc	\
	  image-dispatch.cc image-exr.cc image-io.cc image-jpeg.cc	\
	  image-pfm.cc image-ppm.cc image-png.cc image-rgbe.cc

COMMON_SRCS = cmdlineparser.cc color.cc string-funs.cc $(IMAGE_SRCS)

RENDER_SRCS = brdf.cc camera.cc cook-torrance.cc cubetex.cc far-light.cc \
	  freelist.cc glass.cc glow.cc intersect.cc lambert.cc light.cc	 \
	  lsamples.cc material.cc mesh.cc mirror.cc surface.cc phong.cc	 \
	  point-light.cc primary-surface.cc ray.cc rect-light.cc	 \
	  texture2.cc scene.cc scene-load.cc scene-load-aff.cc space.cc	 \
	  sphere.cc tessel.cc tessel-param.cc timeval.cc trace-state.cc	 \
	  tripar.cc octree.cc

SCENE_DEF_SRCS = scene-def.cc test-scenes.cc

################################################################
##
## Snogray
##

SNOGRAY_SRCS = snogray.cc scene-stats.cc $(SCENE_DEF_SRCS)	\
	       $(RENDER_SRCS) $(COMMON_SRCS)

SNOGRAY_OBJS = $(SNOGRAY_SRCS:.cc=.o)

snogray: $(SNOGRAY_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGRAY_OBJS) $(LIBS)

################################################################
##
## snogsamp (debugging utility)
##

SNOGSAMP_SRCS = snogsamp.cc sample-map.cc $(SCENE_DEF_SRCS)	\
		$(RENDER_SRCS) $(COMMON_SRCS)
SNOGSAMP_OBJS = $(SNOGSAMP_SRCS:.cc=.o)

snogsamp: $(SNOGSAMP_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGSAMP_OBJS) $(LIBS)

################################################################
##
## snogcvt/snogdiff (image utilities)
##

SNOGCVT_SRCS = snogcvt.cc $(COMMON_SRCS)
SNOGCVT_OBJS = $(SNOGCVT_SRCS:.cc=.o)

snogcvt: $(SNOGCVT_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGCVT_OBJS) $(LIBS)

SNOGDIFF_SRCS = snogdiff.cc $(COMMON_SRCS)
SNOGDIFF_OBJS = $(SNOGDIFF_SRCS:.cc=.o)

snogdiff: $(SNOGDIFF_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGDIFF_OBJS) $(LIBS)

################################################################
##
## Union of all source/object files, used in cleaning
##

ALL_SRCS = $(sort $(SNOGRAY_SRCS) $(SNOGCVT_SRCS) $(SNOGDIFF_SRCS)	\
		  $(SNOGSAMP_SRCS))
ALL_OBJS = $(sort $(SNOGRAY_OBJS) $(SNOGCVT_OBJS) $(SNOGDIFF_OBJS)	\
		  $(SNOGSAMP_OBJS))

################################################################
##
## Special build rules for image backends (mainly to supplement the
## include path)
## 

image-exr.o: image-exr.cc
	$(CXX) -c $(LIBEXR_CFLAGS) $(_CXXFLAGS) $<
image-png.o: image-png.cc
	$(CXX) -c $(LIBPNG_CFLAGS) $(_CXXFLAGS) $<
image-jpeg.o: image-jpeg.cc
	$(CXX) -c $(LIBJPEG_CFLAGS) $(_CXXFLAGS) $<
image-ppm.o: image-ppm.cc
	$(CXX) -c $(LIBNETPBM_CFLAGS) $(_CXXFLAGS) $<

################################################################
##
## Automatic dependency generation using gcc-generated .d files
##

DEPS = $(ALL_SRCS:%.cc=.%.d)
-include $(DEPS)

################################################################
##
## Our basic build rules
##

%.o: %.cc
	$(CXX) -c $(_CXXFLAGS) $<
%.s: %.cc
	$(CXX) -S $(filter-out -frepo -g,$(_CXXFLAGS)) $<

%.o: %.c
	$(CC) -c $(_CFLAGS) $<
%.s: %.c
	$(CC) -S $(_CFLAGS) $<


# These are generated by g++ -frepo
#
RPO_FILES = $(ALL_OBJS:%.o=%.rpo) 
CLEAN_RPO_FILES = $(if $(filter -frepo,$(CXXFLAGS)),$(RPO_FILES))

clean:
	$(RM) $(TARGETS) $(ALL_OBJS) $(CLEAN_RPO_FILES) $(DEPS)

# Handy for seeing what options are being generated...
#
cflags:
	@echo "CFLAGS = $(CFLAGS)"
	@echo "MACHINE_CFLAGS = $(MACHINE_CFLAGS)"
	@echo "HOST_CFLAGS_$(HOST) = $(HOST_CFLAGS_$(HOST))"
	@echo "ARCH_CFLAGS_$(ARCH) = $(ARCH_CFLAGS_$(ARCH))"
	@echo "ARCH = $(ARCH)"
	@echo "HOST = $(HOST)"

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
