# This isn't a working Makefile.am, these are just fragments from the
# old top-level file.

EXTRA_DIST = stats.txt spd.txt

# snogsamp is broken now
bin_PROGRAMS += snogsamp

# hemint is rarely used, so don't compile it
noinst_PROGRAMS += hemint
snogsamp_SOURCES = snogsamp.cc sample-map.cc sample-map.h
snogsamp_LDADD = $(LOAD_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)

hemint_SOURCES = hemint.cc
hemint_LDADD = util/libsnogutil.a
