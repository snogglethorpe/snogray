################################################################
#
# General compilation flags.  "OPT" and "DEBUG" intentionally have
# short names so that it's easy for a user to override them from the
# command line.  CXX_OPT_FLAGS, CXX_OPT_SIZE_FLAGS, CXX_MACH_FLAGS,
# and CXX_EXTRA_OPT_FLAGS are set by configure.
#
OPT = $(CXX_OPT_FLAGS) $(CXX_EXTRA_OPT_FLAGS) $(CXX_MACH_FLAGS)
OPT_SIZE = $(CXX_OPT_SIZE_FLAGS) $(CXX_EXTRA_OPT_FLAGS) $(CXX_MACH_FLAGS)
DEBUG = -g

# Set the default compiler options from OPT and DEBUG.  If the user
# wants to change the optimization or debug settings, he should set
# one of those on the make command line.  CXX_STD_FLAGS,
# CXX_CHECK_FLAGS, and EXTRA_COMPILE_FLAGS are set by configure.
#
# [Note that using CXX_CHECK_FLAGS in AM_CFLAGS, which is used by the
# C compiler, is a little hacky, but usually works in practice as the
# C and C++ compilers are usually the "same" compiler and so accept
# mostly the same options.  If this turns out not to work for some
# compiler, a separate "C_CHECK_FLAGS" variable could be introduced
# which contains check options specifically for the C compiler.]
#
AM_CFLAGS = $(OPT) $(DEBUG) $(CXX_CHECK_FLAGS) $(EXTRA_COMPILE_FLAGS)
AM_CXXFLAGS = $(OPT) $(DEBUG) $(CXX_STD_FLAGS) $(CXX_CHECK_FLAGS) $(EXTRA_COMPILE_FLAGS)

# Versions of AM_CFLAGS and AM_CXXFLAGS for situations where we want to
# optimize for size.
#
AM_CFLAGS_SIZE = $(OPT_SIZE) $(DEBUG) $(EXTRA_COMPILE_FLAGS)
AM_CXXFLAGS_SIZE = $(OPT_SIZE) $(DEBUG) $(CXX_STD_FLAGS) $(CXX_CHECK_FLAGS) $(EXTRA_COMPILE_FLAGS)

# These options are required for correctness, and _must_ come _after_
# any optimization options (gcc isn't very bright about parsing
# options:  later options always override earlier ones, even if the
# earlier one was more specific).
#
AM_CFLAGS += $(CXX_EXTRA_REQ_FLAGS)
AM_CXXFLAGS += $(CXX_EXTRA_REQ_FLAGS)

# Preprocessor flags.  When we build in subdirectories, we need to
# find include files relative to our source root.
#
AM_CPPFLAGS = -I$(top_srcdir)

# Add compiler flags (nominally "cflags", but in practice always
# preprocessor options) needed by libraries we use.
#
# It would be nice to only use these flags when compiling the actual
# object file which needs them -- however automake has very poor support
# for specifying non-global compiler options (even where it can be done,
# automake's implementation has annoying side-effects).
#
AM_CPPFLAGS += $(libpng_CFLAGS) $(libexr_CFLAGS) $(libjpeg_CFLAGS)	\
	$(libnetpbm_CFLAGS) $(LIB3DS_CFLAGS) $(liblua_CFLAGS)


################################################################
#
# Linker flags
#
AM_LDFLAGS = $(CONFIG_LDFLAGS)
