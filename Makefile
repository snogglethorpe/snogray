TARGETS = test

all: $(TARGETS)

CXXFLAGS = -O2 $(DEP_CFLAGS)

DEP_CFLAGS = -MMD -MF $(<:%.cc=.%.d)

CXXSRCS = test.cc mray.cc scene.cc obj.cc intersect.cc color.cc sphere.cc

OBJS = $(CXXSRCS:.cc=.o)

test: $(OBJS)
	$(CXX) -o $@ $(OBJS)

DEPS = $(CXXSRCS:%.cc=.%.d)
-include $(DEPS)

%.s: %.cc
	$(CXX) -S $(CFLAGS) $<

clear:
	$(RM) $(TARGETS) $(OBJS)

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
