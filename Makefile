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

#OPT = -O5
DEBUG = -g
#PG = -pg
#MUDFLAP = -fmudflap

CFLAGS = $(OPT) $(DEBUG) $(MACHINE_CFLAGS) $(PG) $(MUDFLAP)
LDFLAGS = $(PG) $(MUDFLAP)

HOST_CFLAGS_dhapc248.dev.necel.com = $(ARCH_CFLAGS_pentium4)
ARCH_CFLAGS_pentium3 = -march=pentium3 -fomit-frame-pointer
ARCH_CFLAGS_pentium4 = -march=pentium4 -fomit-frame-pointer
ARCH_CFLAGS_i686 = $(ARCH_CFLAGS_pentium3)

CXXFLAGS = $(CFLAGS)
DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

HOST := $(shell hostname)
ARCH := $(shell uname -m)
HOST_CFLAGS_$(HOST) ?= $(ARCH_CFLAGS_$(ARCH))
MACHINE_CFLAGS = $(HOST_CFLAGS_$(HOST))

PNG_LIBS	= -lpng

EXR_CFLAGS	= -I/usr/include/OpenEXR
EXR_LIBS	= -lIlmImf -lIex -lHalf

LIBS = $(PNG_LIBS) $(EXR_LIBS) $(MUDFLAP:-f%=-l%)

_CFLAGS = $(CFLAGS) $(DEP_CFLAGS)
_CXXFLAGS = $(CXXFLAGS) $(DEP_CFLAGS)

SNOGRAY_SRCS = camera.cc cmdlineparser.cc color.cc glow.cc image.cc	 \
	  image-cmdline.cc image-exr.cc image-png.cc intersect.cc	 \
	  lambert.cc obj.cc phong.cc ray.cc scene.cc snogray.cc space.cc \
	  sphere.cc triangle.cc voxtree.cc
SNOGRAY_OBJS = $(SNOGRAY_SRCS:.cc=.o)

snogray: $(SNOGRAY_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGRAY_OBJS) $(LIBS)

SNOGCVT_SRCS = cmdlineparser.cc color.cc image.cc image-cmdline.cc	\
	  image-exr.cc image-png.cc snogcvt.cc
SNOGCVT_OBJS = $(SNOGCVT_SRCS:.cc=.o)

snogcvt: $(SNOGCVT_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(SNOGCVT_OBJS) $(LIBS)

# OpenEXR include files expect their include directory to be in the include path
image-exr.o: image-exr.cc
	$(CXX) -c $(EXR_CFLAGS) $(_CXXFLAGS) $<

DEPS = $(SNOGRAY_SRCS:%.cc=.%.d)
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
	$(RM) $(TARGETS) $(SNOGRAY_OBJS)

cflags:
	@echo "CFLAGS = $(CFLAGS)"
	@echo "MACHINE_CFLAGS = $(MACHINE_CFLAGS)"
	@echo "HOST_CFLAGS_$(HOST) = $(HOST_CFLAGS_$(HOST))"
	@echo "ARCH_CFLAGS_$(ARCH) = $(ARCH_CFLAGS_$(ARCH))"
	@echo "ARCH = $(ARCH)"
	@echo "HOST = $(HOST)"

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
