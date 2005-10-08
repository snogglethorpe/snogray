TARGETS = test

all: $(TARGETS)

CFLAGS = -O2 -g
CXXFLAGS = $(CFLAGS)
DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

LIBS = -lpng

_CFLAGS = $(CFLAGS) $(DEP_CFLAGS)
_CXXFLAGS = $(CXXFLAGS) $(DEP_CFLAGS)

CXXSRCS = test.cc snogray.cc scene.cc obj.cc intersect.cc color.cc	\
	  lambert.cc sphere.cc camera.cc space.cc voxtree.cc ray.cc	\
	  image.cc triangle.cc phong.cc

OBJS = $(CXXSRCS:.cc=.o)

test: $(OBJS)
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
