# Automake Makefile template for "raw" SWIG interface to snogray, libsnograw.a
#
#  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

include $(top_srcdir)/config.mk
include $(top_srcdir)/image/image-deps.mk


noinst_LIBRARIES = libsnograw.a

EXTRA_DIST = common.swg snograw.swg

CLEANFILES = snograw.so

# We only clean up "snograw_lua_wrap.cc" if it's possible to
# regenerate it.
#
if have_swig
CLEANFILES += snograw_lua_wrap.cc
endif


AM_CPPFLAGS += $(liblua_CFLAGS) $(libsnogimage_CPPFLAGS)


libsnograw_a_SOURCES = snograw_lua_wrap.cc

# We include some other SWIG files from snograw.swg, so reflect those
# dependencies.
#
snograw_lua_wrap.cc: common.swg $(top_srcdir)/scene/camera.swg		\
	$(top_srcdir)/color/color.swg $(top_srcdir)/image/image.swg	\
	$(top_srcdir)/light/light.swg $(top_srcdir)/load/load.swg	\
	$(top_srcdir)/lua-util/lua-val-table.swg			\
	$(top_srcdir)/material/material.swg				\
	$(top_srcdir)/render-mgr/render-mgr.swg				\
	$(top_srcdir)/render/render.swg $(top_srcdir)/scene/scene.swg	\
	$(top_srcdir)/space/space.swg					\
	$(top_srcdir)/surface/surface.swg				\
	$(top_srcdir)/texture/texture.swg $(top_srcdir)/util/util.swg	\
	$(top_srcdir)/util/lua-vector.swg


# Compiler options for compiling SWIG-generated Lua interface.
#
# These are the same as the default compiler options, except:
# (1) Optimize for size (SWIG-generated code can be bloated)
# (2) Add "-fno-strict-aliasing", as the generated code uses some
#     dodgy aliasing tricks
# (3) Don't use pedantic checking options (the quality of the
#     generated code is not something we can easily control)
#
libsnograw_a_CXXFLAGS := $(OPT_SIZE) $(DEBUG) $(CXX_STD_FLAGS)		\
	-fno-strict-aliasing $(CXX_EXTRA_REQ_FLAGS) $(EXTRA_COMPILE_FLAGS)

# Make automake suck slightly less
libsnograw_a_SHORTNAME = sl


# If the swig interface generator is available, we can update the C++
# interface when snograw.swg changes; otherwise we need to use
# pregenerated copies.
#
if have_swig

SWIG_FLAGS = -I$(top_builddir) -I$(top_srcdir) $($(SWIG_NOMODULEGLOBAL_FLAG)

# automake "silent-rules" hooks for swig.
#
swig_verbose = $(swig_verbose_$(V))
swig_verbose_ = $(swig_verbose_$(AM_DEFAULT_VERBOSITY))
swig_verbose_0 = @echo "  SWIG    " $@;

# Build rules for swig-generated files we use.
#
%_lua_wrap.cc : %.swg
	$(swig_verbose)$(SWIG) $(SWIG_FLAGS) -o $@ -c++ -lua $<

else

# A dummy build rule for swig-generated files that gets used when the
# swig program isn't available; it just prints an warning/error
# message when make tries to regenerate the file (which normally
# shouldn't be necessary with a tarball distribution, as
# swig-generated files are distributed in the tarball).
#
%_wrap.cc : %.swg
	@if test -r "$@"; then						       \
	  echo '** Warning: $< is newer than $@, but cannot run swig interface generator'; \
	else								       \
	  echo '** Error: cannot run swig interface generator to generate $@'; \
	  exit 1;							       \
	fi

endif


## Build a dynamically loadable Lua module, for debugging.
##
snograw_so_LIBS = $(LOAD_LIBS) $(CORE_LIBS) $(IMAGE_LIBS) $(MISC_LIBS)
snograw_so_DEPS = $(filterout, -l%, $(snograw_so_LIBS)
snograw.so: $(snograw_DEPS)
	$(CXX) -o $@ --shared -Wl,-uluaopen_snograw libsnograw.a \
		$(snograw_so_LIBS)

