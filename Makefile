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

OPT = -O5
DEBUG = -g -Wall
#PG = -pg
#MUDFLAP = -fmudflap

CFLAGS = $(OPT) $(DEBUG) $(MACHINE_CFLAGS) $(PG) $(MUDFLAP)
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(PG) $(MUDFLAP)

HOST_CFLAGS_dhapc248.dev.necel.com = $(ARCH_CFLAGS_pentium4)
ARCH_CFLAGS_pentium3 = -march=pentium3 -fomit-frame-pointer -mfpmath=sse -msse
ARCH_CFLAGS_pentium4 = -march=pentium4 -fomit-frame-pointer -mfpmath=sse -msse2
ARCH_CFLAGS_i686 = $(ARCH_CFLAGS_pentium3)

HOST := $(shell hostname)
ARCH := $(shell uname -m)
HOST_CFLAGS_$(HOST) ?= $(ARCH_CFLAGS_$(ARCH))
MACHINE_CFLAGS = $(HOST_CFLAGS_$(HOST))

LIBPNG_CFLAGS	= $(shell libpng-config --cflags)
LIBPNG_LIBS	= $(shell libpng-config --libs)

LIBEXR_CFLAGS	= $(shell pkg-config OpenEXR --cflags)
LIBEXR_LIBS	= $(shell pkg-config OpenEXR --libs)

JPEG_CFFLAGS	=
JPEG_LIBS	= -ljpeg

LIBS = $(LIBPNG_LIBS) $(LIBEXR_LIBS) $(JPEG_LIBS) $(MUDFLAP:-f%=-l%)

DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

_CFLAGS_FILT = $(if $(filter -pg,$(CFLAGS)),$(filter-out -fomit-frame-pointer,$(CFLAGS)),$(CFLAGS))
_CXXFLAGS_FILT = $(if $(filter -pg,$(CXXFLAGS)),$(filter-out -fomit-frame-pointer,$(CXXFLAGS)),$(CXXFLAGS))

_CFLAGS = $(_CFLAGS_FILT) $(DEP_CFLAGS)
_CXXFLAGS = $(_CXXFLAGS_FILT) $(DEP_CFLAGS)

#

IMAGE_SRCS = image.cc image-aa.cc image-cmdline.cc image-exr.cc	\
	  image-jpeg.cc image-png.cc image-rgb-byte.cc

COMMON_SRCS = cmdlineparser.cc color.cc $(IMAGE_SRCS)

#

SNOGRAY_SRCS = camera.cc glow.cc intersect.cc lambert.cc material.cc	\
	  obj.cc phong.cc ray.cc scene.cc snogray.cc space.cc sphere.cc	\
	  test-scenes.cc triangle.cc voxtree.cc $(COMMON_SRCS)

SNOGRAY_OBJS = $(SNOGRAY_SRCS:.cc=.o)

snogray: $(SNOGRAY_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGRAY_OBJS) $(LIBS)

#

SNOGCVT_SRCS = snogcvt.cc $(COMMON_SRCS)

SNOGCVT_OBJS = $(SNOGCVT_SRCS:.cc=.o)

snogcvt: $(SNOGCVT_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGCVT_OBJS) $(LIBS)

#

# OpenEXR include files expect their include directory to be in the include path
image-exr.o: image-exr.cc
	$(CXX) -c $(LIBEXR_CFLAGS) $(_CXXFLAGS) $<
image-png.o: image-png.cc
	$(CXX) -c $(LIBPNG_CFLAGS) $(_CXXFLAGS) $<
image-jpeg.o: image-jpeg.cc
	$(CXX) -c $(LIBJPEG_CFLAGS) $(_CXXFLAGS) $<

ALL_SRCS = $(sort $(SNOGRAY_SRCS) $(SNOGCVT_SRCS))
ALL_OBJS = $(sort $(SNOGRAY_OBJS) $(SNOGCVT_OBJS))

DEPS = $(ALL_SRCS:%.cc=.%.d)
-include $(DEPS)

%.o: %.cc
	$(CXX) -c $(_CXXFLAGS) $<
%.s: %.cc
	$(CXX) -S $(_CXXFLAGS) $<

%.o: %.c
	$(CC) -c $(_CFLAGS) $<
%.s: %.c
	$(CC) -S $(_CFLAGS) $<

clean:
	$(RM) $(TARGETS) $(ALL_OBJS)

cflags:
	@echo "CFLAGS = $(CFLAGS)"
	@echo "MACHINE_CFLAGS = $(MACHINE_CFLAGS)"
	@echo "HOST_CFLAGS_$(HOST) = $(HOST_CFLAGS_$(HOST))"
	@echo "ARCH_CFLAGS_$(ARCH) = $(ARCH_CFLAGS_$(ARCH))"
	@echo "ARCH = $(ARCH)"
	@echo "HOST = $(HOST)"

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
