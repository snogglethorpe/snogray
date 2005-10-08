# Makefile for snogray
#
#  Copyright (C) 2005  Miles Bader <miles@gnu.org>
#
# This file is subject to the terms and conditions of the GNU General
# Public License.  See the file COPYING in the main directory of this
# archive for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

TARGETS = snogray snogcvt

all: $(TARGETS)

# Build directory for SPD (Standard Procedural Databases) package by
# Eric Haines; used to generate test scenes.
# See:  http://www.acm.org/tog/resources/SPD
#
SPD_DIR = ../spd3_13

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

JPEG_CFFLAGS	=
JPEG_LIBS	= -ljpeg

LIBS = $(LIBPNG_LIBS) $(LIBEXR_LIBS) $(JPEG_LIBS) $(MUDFLAP:-f%=-l%)

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
## Automatically generated test scenes from .nff files (from spd project)
##

SPD_TESTS = test-teapot.nff test-balls.nff test-rings.nff test-tetra.nff \
	test-mount.nff test-tree.nff test-gears.nff test-sombrero.nff	 \
	test-sample.nff test-jacks.nff test-shells.nff test-nurbtst.nff	 \
	test-lattice.nff test-f117.nff test-skull.nff test-f15.nff	 \
	test-teapot-14.nff test-teapot-30.nff

# C++ source files generated from the .nff files
comma = ,
SPD_TEST_SRCS = $(SPD_TESTS:%.nff=$(comma)%.cc)

# All depend on the conversion program
$(SPD_TEST_SRCS): nff-cvt

################################################################
##
## Common sources between snogray and snogcvt
##

IMAGE_SRCS = image.cc image-aa.cc image-cmdline.cc image-exr.cc	\
	  image-jpeg.cc image-png.cc image-rgb-byte.cc

COMMON_SRCS = cmdlineparser.cc color.cc $(IMAGE_SRCS)

################################################################
##
## Snogray
##

TEST_SRCS = test-scenes.cc test-scene.cc $(SPD_TEST_SRCS)

SNOGRAY_SRCS = camera.cc glow.cc intersect.cc lambert.cc light-model.cc	\
	  material.cc mesh.cc mirror.cc obj.cc phong.cc primary-obj.cc	\
	  ray.cc scene.cc snogray.cc space.cc sphere.cc timeval.cc	\
	  trace-state.cc triangle.cc voxtree.cc $(COMMON_SRCS)		\
	  $(TEST_SRCS)

SNOGRAY_OBJS = $(SNOGRAY_SRCS:.cc=.o)

snogray: $(SNOGRAY_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGRAY_OBJS) $(LIBS)

################################################################
##
## Snogcvt (image conversion program)
##

SNOGCVT_SRCS = snogcvt.cc $(COMMON_SRCS)

SNOGCVT_OBJS = $(SNOGCVT_SRCS:.cc=.o)

snogcvt: $(SNOGCVT_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGCVT_OBJS) $(LIBS)

################################################################
##
## Union of all source/object files, used in cleaning
##

ALL_SRCS = $(sort $(SNOGRAY_SRCS) $(SNOGCVT_SRCS))
ALL_OBJS = $(sort $(SNOGRAY_OBJS) $(SNOGCVT_OBJS))

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

################################################################
##
## Automatic dependency generation using gcc-generated .d files
##

DEPS = $(ALL_SRCS:%.cc=.%.d)
-include $(DEPS)

################################################################
##
## Build rules for making C++ sources from .nff files, and generating
## the .nff files themselves using the "spd" package.
##

test-balls-5.nff: $(SPD_DIR)/balls
	$(SPD_DIR)/balls -s 5 > $@
test-teapot-14.nff: $(SPD_DIR)/teapot
	$(SPD_DIR)/teapot -s 14 > $@
test-teapot-22.nff: $(SPD_DIR)/teapot
	$(SPD_DIR)/teapot -s 22 > $@
test-teapot-30.nff: $(SPD_DIR)/teapot
	$(SPD_DIR)/teapot -s 30 > $@

test-%.nff: $(SPD_DIR)/%
	$< > $@
test-%.nff: $(SPD_DIR)/%.obj
	$(SPD_DIR)/readobj -f $< > $@
test-%.nff: $(SPD_DIR)/%.dxf
	$(SPD_DIR)/readdxf -f $< > $@

,%.cc: %.nff
	nff-cvt $* < $< > $@

################################################################
##
## Our basic build rules
##

%.o: %.cc
	$(CXX) -c $(_CXXFLAGS) $<
%.s: %.cc
	$(CXX) -S $(_CXXFLAGS) $<

%.o: %.c
	$(CC) -c $(_CFLAGS) $<
%.s: %.c
	$(CC) -S $(_CFLAGS) $<

clean:
	$(RM) $(TARGETS) $(ALL_OBJS) $(DEPS)

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
