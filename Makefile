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

TARGETS = snogray

all: $(TARGETS)

#CFLAGS = -O2 -g
#CFLAGS = -O5 -g -march=pentium4 -mfpmath=sse
CFLAGS = -O5 -g -march=pentium3
CXXFLAGS = $(CFLAGS)
DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

LIBS = -lpng

_CFLAGS = $(CFLAGS) $(DEP_CFLAGS)
_CXXFLAGS = $(CXXFLAGS) $(DEP_CFLAGS)

CXXSRCS = snogray.cc scene.cc obj.cc intersect.cc color.cc lambert.cc	\
	  sphere.cc camera.cc space.cc voxtree.cc ray.cc image.cc	\
	  triangle.cc phong.cc cmdlineparser.cc glow.cc

OBJS = $(CXXSRCS:.cc=.o)

snogray: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

DEPS = $(CXXSRCS:%.cc=.%.d)
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
	$(RM) $(TARGETS) $(OBJS)

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
