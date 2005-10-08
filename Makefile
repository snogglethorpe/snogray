CFLAGS = -O2

OBJS = tvec.o tvec2.o

tvec: $(OBJS)
	$(CXX) -o $@ $(OBJS)

$(OBJS): vec.h

%.s: %.cc
	$(CXX) -S $(CFLAGS) $<

# arch-tag: cfcae754-60d5-470f-b3ea-248fbf0a01c8
