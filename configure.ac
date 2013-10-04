# Autoconf configuration for snogray
#
#  Copyright (C) 2006-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

AC_INIT([snogray], [0.5.3], [Miles Bader <miles@gnu.org>])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign subdir-objects std-options])


# Set CFLAGS and CXXFLAGS to suppress AC_PROG_CC's attempt to set
# them, preserving anything the user specifies.  [We do this because
# we attempt to set compiler options in a much finer-grained manner,
# which makes autoconf's defaults redundant.]
#
# For good measure, do the same thing for CPPFLAGS and LDFLAGS as
# well.
#
CFLAGS=${CFLAGS:-""}
CXXFLAGS=${CXXFLAGS:-""}
CPPFLAGS=${CPPFLAGS:-""}
LDFLAGS=${LDFLAGS:-""}

# During configuration, various compiler options may be added to
# CFLAGS / CXXFLAGS / CPPFLAGS / LDFLAGS, so that any subsequent
# configuration compiles will be done with compiler options matching
# what will be used for building.
#
# However during building, those added options will be set using other
# Make variables, so to avoid duplicating the added options, save the
# original state of these variable (containing only what the user
# specified on the configure command-line), so we can restore them
# just before creating the Makefile.
#
USER_CFLAGS=$CFLAGS
USER_CXXFLAGS=$CXXFLAGS
USER_CPPFLAGS=$CPPFLAGS
USER_LDFLAGS=$LDFLAGS


AC_PROG_CC
AC_PROG_CXX

AC_PROG_RANLIB

AC_PROG_AWK

AC_CHECK_PROG([SWIG], [swig], [swig])


# We use C++ for everything, so do all our tests using that too.
#
AC_LANG(C++)


# Use terse build output by default (can be disabled at
# compile time with "make V=0" or at configure time with
# "configure --disable-silent-rules")
#
AM_SILENT_RULES([yes])


# This is a fake "conditional" we use for commenting out stuff in
# Makefile.am
#
AM_CONDITIONAL([never], [false])


# Enable/disable double-precision floating-point coordinates
#
# single-precision (the default) saves memory, and on some hardware is
# simply faster than double-precision, but can result in precision
# errors for some scenes (typically, though, this can be worked around
# at runtime by increasing the value of the "min-trace" rendering
# parameter).
#
AC_ARG_ENABLE([double-coords],
    AS_HELP_STRING([--enable-double-coords],
		   [Use double-precision coordinates]),
    [enable_double_coords="$enableval"],
    [enable_double_coords=no])
AC_MSG_CHECKING([whether to use double-precision coordinates])
AC_MSG_RESULT([$enable_double_coords])
if test "$enable_double_coords" = yes; then
  AC_DEFINE([USE_DOUBLE_COORDS], [1],
  	    [Define if using double-precision coordinates])
fi


# Enable/disable link-time optimization.  We default to "yes", and test
# whether it actually works later in this file.
#
AC_ARG_ENABLE([lto],
    AS_HELP_STRING([--enable-lto],
		   [Enable link-time optimization]),
    [lto="$enableval"],
    [lto="yes"])


# The variable containing our linker flags.  This is addition to
# LDFLAGS, which is reserved for the user.
#
AC_SUBST([CONFIG_LDFLAGS], [])


##
## --------------------------------
## Optional addons
##


# --with-lua-src=DIR option
#
# The rules for choosing which Lua library to use are:
#
#   (1) If the user specifies --with-lua-src=DIR, then a Lua library
#       will be built from the source in DIR and used (and no system
#       Lua library will be checked for).
#
#   (2) If the user specifies --with-lua-src, then a Lua library will
#       be built from the source in $top_srcdir/liblua/lua-dist and
#       used (and no system Lua library will be checked for).
#
#   (3) If --with-lua-src isn't specified, and a system LuaJIT or Lua
#       library is found, then the system library is used.  A LuaJIT
#       library is preferred to a Lua library.
#
#   (4) If --with-lua-src isn't specified and no system LuaJIT or Lua
#       library is found, then a Lua library is built from source code
#       $top_srcdir/liblua/lua-dist and used.
#
# In any case where it attempts to build from Lua source code, it is a
# fatal error if no source code is found in the selected location.
#

lua_dist=default
AC_ARG_WITH([lua-src],
    AS_HELP_STRING([--with-lua-src], [Specify Lua source directory]),
    [lua_dist=$withval])

if test x"$lua_dist" = xdefault; then
  lua_dist=yes
  build_lua_from_src=maybe
else
  build_lua_from_src=yes
fi
if test x"$lua_dist" = xyes; then
  lua_dist=$srcdir/liblua/lua-dist
  lua_dist_make='$(top_srcdir)/liblua/lua-dist'
else
  lua_dist_make="$lua_dist"
fi

AC_MSG_CHECKING([for Lua source directory])
lua_dist=`echo "$lua_dist" | sed -e 's@^./@@' -e 's@/*$@@'`
lua_dist_make=`echo "$lua_dist_make" | sed -e 's@^./@@' -e 's@/*$@@'`
lua_src=none
for sfx in '' '/src'; do
    if test -r "$lua_dist$sfx/lapi.c"; then
       lua_src="$lua_dist$sfx"
       lua_src_make="$lua_dist_make$sfx"
       break
    fi
done
AC_MSG_RESULT([$lua_src])

if test x"$lua_src" = xnone && test $build_lua_from_src = yes; then
  AC_MSG_ERROR([no Lua source found in directory "$lua_dist"; aborting...])
fi

build_lua_in_src_dir=no
if test "$lua_src" != none; then
  AC_SUBST([LUA_DIST], [$lua_dist_make])
  AC_SUBST([LUA_SRC], [$lua_src_make])

  # We use two different methods for building liblua.a, depending on
  # whether we're using a separate build directory or not, and whether
  # the Lua source directory is a subdirectory (and thus is "ours"):
  #
  # + If building in the source dir, and the Lua source directory is a
  #   simple subdirectory of the top-level source dir, we build
  #   liblua.a in the Lua source directory and copy it to our
  #   top-level directory.  This avoids most assumptions about how
  #   make works.
  #
  # + If using a separate build dir, or if the Lua source directory is
  #   "somewhere else", we build liblua.a in the current directory,
  #   using the Makefile from the Lua source directory, and VPATH to
  #   tell make where to find the source files. This requires that
  #   make support at using VPATH to find source files, which seems a
  #   reasonable assumption in this case.
  #
  # Our test to determine if LUA_SRC is "our" subdirectory is very
  # simple, we just see if $lua_dist _doesn't_ contain a slash (we use
  # $lua_dist instead of $lua_src because $lua_src is often a
  # subdirectory of $lua_dist, and so will have a slash added).
  #
  AC_MSG_CHECKING([whether to build Lua in source directory])
  lua_dist_slash=`echo "$lua_dist" | grep /`
  if test x"$srcdir" = x. && test x"$lua_dist_slash" != x"$lua_dist"; then
    build_lua_in_src_dir=yes
  fi
  AC_MSG_RESULT([$build_lua_in_src_dir])
fi

AM_CONDITIONAL([build_lua_in_src_dir], [test $build_lua_in_src_dir = yes])

##
## --------------------------------
## Compiler tests
##


# Test whether the compiler accepts various flags

# Individually checks whether the C++ compiler accepts the whitespace
# separated flags in $2, and defines a subst named $1 containing all
# options that were accepted.
#
AC_DEFUN([SNOGRAY_CHECK_CXX_FLAGS], [
    AC_LANG_ASSERT(C++)
    _SAVED_CXXFLAGS="$CXXFLAGS"
    OK_FLAGS=""
    for OPT in $2; do
      CXXFLAGS="$_SAVED_CXXFLAGS $OPT"
      AC_MSG_CHECKING(whether C++ compiler accepts "$OPT" option)
      AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE([int x;])],
        [opt_ok=yes
	 # Some compilers only warn about unsupported options, so
	 # try to detect such warnings.
	 if test -s conftest.err && grep ".*$OPT" conftest.err >/dev/null; then
	   opt_ok=no
	 fi],
	[opt_ok=no])
      AC_MSG_RESULT([$opt_ok])
      if test "$opt_ok" = yes; then
	if test "$OK_FLAGS" = ""; then
	  OK_FLAGS="$OPT"
	else
	  OK_FLAGS="$OK_FLAGS $OPT"
	fi
      fi
    done
    AC_SUBST([$1], [$OK_FLAGS])
    CXXFLAGS="$_SAVED_CXXFLAGS"
])

# Checks whether the C+++ compiler accepts the flags in $2, and if so
# defines a subst named $1 containing $2.
#
AC_DEFUN([SNOGRAY_CHECK_CXX_FLAG], [
    AC_LANG_ASSERT(C++)
    _SAVED_CXXFLAGS="$CXXFLAGS"
    OPT="$2"
    CXXFLAGS="$CXXFLAGS $OPT"
    case "$OPT" in *' '*) S=s;; *) S="";; esac
    AC_MSG_CHECKING([whether C++ compiler accepts "$OPT" option$S])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE([int x;])],
      [opt_ok=yes
       # Some compilers only warn about unsupported options, so
       # try to detect such warnings.
       if test -s conftest.err; then
         for ONE_OPT in $OPT; do
	   if grep ".*$ONE_OPT" conftest.err >/dev/null; then
	     opt_ok=no
	     break
	   fi
	 done
       fi],
      [opt_ok=no])
    AC_MSG_RESULT([$opt_ok])
    if test "$opt_ok" = yes; then
      AC_SUBST([$1], ["$OPT"])
    fi
    CXXFLAGS="$_SAVED_CXXFLAGS"
])

# Checks whether the C++ compiler accepts the linker flags in $2,
# and if so defines a subst named $1 containing $2.
#
# Note that $2 is still passed to the compiler driver for linking, so
# it should use whatever syntax is necessary (e.g., for gcc, a "-Wl,"
# prefix is usually used to pass flags to the linker).
#
AC_DEFUN([SNOGRAY_CHECK_CXX_LINK_FLAG], [
    AC_LANG_ASSERT(C++)
    _SAVED_LDFLAGS="$LDFLAGS"
    OPT="$2"
    LDFLAGS="$LDFLAGS $OPT"
    case "$OPT" in *' '*) S=s;; *) S="";; esac
    AC_MSG_CHECKING([whether C++ compiler accepts "$OPT" linker option$S])
    AC_LINK_IFELSE(
      [AC_LANG_SOURCE([int main () { return 0; } ])],
      [opt_ok=yes
       # Some compilers only warn about unsupported options, so
       # try to detect such warnings.
       if test -s conftest.err; then
         for ONE_OPT in $OPT; do
	   if grep ".*$ONE_OPT" conftest.err >/dev/null; then
	     opt_ok=no
	     break
	   fi
	 done
       fi],
      [opt_ok=no])
    AC_MSG_RESULT([$opt_ok])
    if test "$opt_ok" = yes; then
      AC_SUBST([$1], ["$OPT"])
    fi
    LDFLAGS="$_SAVED_LDFLAGS"
])


# Try to choose a compiler support level option.
#
broken_cxx11=no
for opt in -std=c++11 -std=c++0x -std=c++98; do
  # See if this is option should enable a C++11 mode.
  cxx11=no
  case "$opt" in "-std=c++0x"|"-std=c++11") cxx11=yes;; esac

  # Don't bother checking a C++11-enabling option if we've already
  # determined C++11 support is broken for this compiler.
  test $cxx11+$broken_cxx11 = yes+yes && continue

  # See if the option works.
  SNOGRAY_CHECK_CXX_FLAG([CXX_STD_FLAGS], [$opt])

  # Some compilers (versions of g++ 4.3, clang with a mismatched
  # version of libstdc++) have broken support for c++11, so try to
  # detect problematic cases we know about, and revert to c++98 if
  # compilation fails.
  #
  if test x"$CXX_STD_FLAGS" != x && test $cxx11 = yes; then
    #
    # This is just a mismash of code containing valid bits of C++11
    # code that fail to compile on various compilers.
    #
    AC_MSG_CHECKING([checking whether c++11 support is broken])
    _OLD_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS $CXX_STD_FLAGS"
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
       [#include <iostream>
	#include <type_traits>
	#include <algorithm>
	#include <utility>

	std::pair<int, int> ip;

	// When compiling with a "newish" libstdc++ which supports
	// constexpr, make sure constexpr actually _works_ because
	// libstdc++ depends on it in c++11 mode.  This issue arises
	// with some clang installations which use libstdc++ from a
	// co-installed gcc, but barf on some libstdc++ includes in
	// c++11 mode.
	#ifdef _GLIBCXX_USE_CONSTEXPR
	  constexpr int const_func () { return 3; }
	  static_assert (const_func() > 0, "constexpr barf");
	#endif

	// This test fails in c++0x mode on broken gcc 4.3 versions
	bool file_exists () { return true; }
	using namespace std;
	bool test () { return file_exists (); }])],
     [broken_cxx11=no],
     [broken_cxx11=yes])
   CXXFLAGS="$_OLD_CXXFLAGS"
   AC_MSG_RESULT([$broken_cxx11])
   if test "$broken_cxx11" = yes; then
     CXX_STD_FLAGS=""
     cxx_level_note=" (c++11 support broken)"
   fi
  fi

  # Stop searching if we found a working option.
  test x"$CXX_STD_FLAGS" = x || break
done
#
# Turn the discovered option into a pretty name
#
cxx_level=none
case "$CXX_STD_FLAGS" in
  "-std="*)
    cxx_level=`echo "$CXX_STD_FLAGS" | sed 's@^-std=@@'`
    ;;
  .*)
    cxx_level="unknown"
    ;;
esac
#
AC_MSG_CHECKING([compiler standard support])
AC_MSG_RESULT([$cxx_level$cxx_level_note])
#
# Put the option in CXXFLAGS so following configure tests use it.
#
CXXFLAGS="$CXXFLAGS $CXX_STD_FLAGS"


# Try to be very picky, if the compiler lets us.
#
# We turn on all standard warnings, plus a few others, and use
# -pedantic-errors to cause an error for standard violations.  However
# we also inhibit warnings about "long long", as we use that too much.
#
SNOGRAY_CHECK_CXX_FLAGS([CXX_CHECK_FLAGS],
  [-Wall -Wextra -Winit-self -Wdouble-promotion -pedantic-errors -Wno-long-long])
#
# Again, update CXXFLAGS so following configure tests use our
# warning/error settings -- some tests may fail with them which don't
# otherwise.
#
CXXFLAGS="$CXXFLAGS $CXX_CHECK_FLAGS"


##
## --------------------------------
## Language feature tests
##


#
# See if this compiler supports the "extern template class Foo<T>"
# syntax for suppressing template instantiation of specific types.
# This is availble in C++11, and also as a g++ extension to C++98.
#

AC_ARG_ENABLE([extern-template],
    AS_HELP_STRING([--disable-extern-template],
		   [Disable use of C++ "extern template" extension]),
    [enable_extern_template="$enableval"],
    [enable_extern_template=yes])

have_extern_template="no"
extern_template_extension_kw=""
if test "$enable_extern_template" = yes; then
  #
  # See if the compiler supports "extern template".
  #
  # If it doesn't seem to work at first, we try again, this time
  # prefixing "extern template" with "__extension__"; this tells g++
  # to allow the feature even in strict compatibility mode.
  #
  for extension_kw in '' '__extension__'; do
    msg_suffix=""
    test x$extension_kw = x__extension__ && msg_suffix=" with $extension_kw"
    AC_MSG_CHECKING([whether C++ compiler supports "extern template"$msg_suffix])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE([template<class T> class Foo { void bar () {} };
	 $extension_kw extern template class Foo<int>;])],
      [have_extern_template=yes],
      [have_extern_template=no])
    AC_MSG_RESULT([$have_extern_template])
    if test $have_extern_template = yes; then
      extern_template_extension_kw="$extension_kw"
      break
    fi
  done
fi

if test $have_extern_template = yes; then
  AC_DEFINE([HAVE_EXTERN_TEMPLATE], [1],
	    [Define if C++ compiler supports "extern template" extension])
  AC_DEFINE_UNQUOTED([EXTERN_TEMPLATE_EXTENSION],
		     [$extern_template_extension_kw],
		     [Prefix used before "extern template" (so that the compiler accepts it even in strict C++98 mode)])
fi


# See if it's valid to cast a function pointer to void* using
# reinterpret_cast.  This is traditionally not allowed, but it seems
# to be OK in C++11, so some compilers may support it.
#
AC_MSG_CHECKING([whether reinterpret_cast<void*> on a function pointer works])
AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE(
     [extern void (*fun_ptr)();
      void *test () { return reinterpret_cast<void*> (fun_ptr); }])],
  [[# Not an error, but let's try to see if it's a warning...
    reinterp_cast_fun_ptr_to_data_ptr_ok=yes
    if test -s conftest.err; then
      if grep '[Ww]arning' conftest.err >/dev/null || \
	grep '[Cc]ast' conftest.err >/dev/null
      then
	reinterp_cast_fun_ptr_to_data_ptr_ok=no
      fi
    fi]],
  [reinterp_cast_fun_ptr_to_data_ptr_ok=no])
AC_MSG_RESULT([$reinterp_cast_fun_ptr_to_data_ptr_ok])
if test "$reinterp_cast_fun_ptr_to_data_ptr_ok" = yes; then
  AC_DEFINE([REINTERP_CAST_FUN_PTR_TO_DATA_PTR_OK], [1],
  	    [Define if reinterpret_cast<void*> works on function pointers])
fi


##
## --------------------------------
## Standard library/environment tests
##


AC_CHECK_HEADERS([stdint.h unistd.h fcntl.h sys/mman.h sys/stat.h])

AC_TYPE_INTPTR_T


# Check for std::trunc in <cmath>, which is available in C++11, but
# not in C++98.
#
have_std_trunc=no
AC_MSG_CHECKING([for std::trunc in <cmath>])
AC_TRY_COMPILE([#include <cmath>],
	       [float x = std::trunc (1.5987f);],
	       [have_std_trunc=yes], [:])
AC_MSG_RESULT([$have_std_trunc])
if test $have_std_trunc = yes; then
  AC_DEFINE([HAVE_STD_TRUNC], [1], [Define if std::trunc is defined in <cmath>])
fi


# Check for std::copysign in <cmath>, which is available in C++11, but
# not in C++98.
#
have_std_copysign=no
AC_MSG_CHECKING([for std::copysign in <cmath>])
AC_TRY_COMPILE([#include <cmath>],
	       [float x = std::copysign (5.f, -1.f);],
	       [have_std_copysign=yes], [:])
AC_MSG_RESULT([$have_std_copysign])
if test $have_std_copysign = yes; then
  AC_DEFINE([HAVE_STD_COPYSIGN], [1], [Define if std::copysign is defined in <cmath>])
fi


# Allow point exceptions to be enabled (which can make debugging much easier).

AC_ARG_ENABLE([fp-exceptions],
    AS_HELP_STRING([--disable-fp-exceptions],
		   [Disable floating-point exceptions]),
    [enable_fp_exceptions="$enableval"],
    [enable_fp_exceptions=yes])

use_fp_exceptions=no

if test "$enable_fp_exceptions" = yes; then

  AC_CHECK_HEADERS([fenv.h])

  have_feenableexcept=no
  if test "$ac_cv_header_fenv_h" = yes; then
    AC_CHECK_LIB([m], [feenableexcept], [have_feenableexcept=yes])

    if test "$have_feenableexcept" = yes; then
      # Clang has some bad interactions with libstdc++ header files that
      # cause it to screw up <fenv.h>, so test for that.
      AC_MSG_CHECKING([whether feenableexcept seems broken])
      AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE(
	   [#include <fenv.h>
	    bool test () { fexcept_t fe; feenableexcept (fe); }])],
	[feenableexcept_broken=no],
	[feenableexcept_broken=yes])
      AC_MSG_RESULT([$feenableexcept_broken])

      if test $feenableexcept_broken = yes; then
	have_feenableexcept=no
      fi
    fi
  fi

  if test $have_feenableexcept = yes; then
    AC_DEFINE([HAVE_FEENABLEEXCEPT], [1],
	      [Define if feenableexcept is defined in -lm])
  fi

  if test "$ac_cv_header_fenv_h" = yes && test "$have_feenableexcept" = yes
  then
    use_fp_exceptions=yes
  fi
fi

AC_MSG_CHECKING([whether to enable floating-point exceptions])
AC_MSG_RESULT([$use_fp_exceptions])
if test "$use_fp_exceptions" = yes; then
  AC_DEFINE([USE_FP_EXCEPTIONS], [1],
  	    [Define if enabling floating-point exceptions])
fi


##
## --------------------------------
## Tests for optional autoconf macros
##


# Check for pkg-config program, used for configuring some libraries.
#
m4_define_default([PKG_PROG_PKG_CONFIG],
  [AC_MSG_CHECKING([pkg-config])
   AC_MSG_RESULT([no])])
PKG_PROG_PKG_CONFIG

# If the pkg-config autoconf support isn't installed, define its
# autoconf macro to disable any packages depending on it.
#
m4_define_default([PKG_CHECK_MODULES],
  [AC_MSG_CHECKING([$1])
   AC_MSG_RESULT([no])
   $4])

# If we're cross-compiling, forcibly disable pkg-config stuff, as
# pkg-config doesn't currently handle cross-compilation properly.
#
if test $cross_compiling = yes && test -n "$PKG_CONFIG"; then
   AC_MSG_WARN([disabling use of pkg-config due to cross-compiling])
   PKG_CONFIG=""
fi


##
## --------------------------------
## External library tests
##


# Tries to find the header file $2 (in C <> or "" syntax) with the
# various include directories specified in $3 (space-separated) added to
# the include path.
#
# If found, then the absolute include directory used is assigned to $1
# and the shell statement in $4 is evaluted; if not found anywhere, $1
# is set to "", and the shell statement in $5 are evaluated.
#
AC_DEFUN([SNOGRAY_CHECK_SUBDIR_HEADER], [
    _SAVED_CPPFLAGS="$CPPFLAGS"
    AC_MSG_CHECKING([for $2 include dir])
    _snogray_check_subdir_header_dir=""
    for _snogray_check_subdir_header_root in "$includedir" "/usr/local/include" "/usr/include"
    do
      for _snogray_check_subdir_header_sfx in $3; do
	_snogray_check_subdir_header_dir="$_snogray_check_subdir_header_root/$_snogray_check_subdir_header_sfx"
	CPPFLAGS="$_SAVED_CPPFLAGS -I$_snogray_check_subdir_header_dir"
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include $2]])], [break 2])
	_snogray_check_subdir_header_dir=""
      done
    done
    CPPFLAGS="$_SAVED_CPPFLAGS"
    _snogray_check_subdir_header_result=
    if test x"$_snogray_check_subdir_header_dir" != x""; then
      _snogray_check_subdir_header_result="$_snogray_check_subdir_header_dir"
      $1="$_snogray_check_subdir_header_dir"
      $4
    else
      _snogray_check_subdir_header_result=none
      $1=""
      $5
    fi
    AC_MSG_RESULT([$_snogray_check_subdir_header_result])
])


# Check for lua library.
#
# First see if we're using our own builtin copy of Lua
#
have_liblua=no
have_luajit=no
if test $build_lua_from_src = yes; then
  # set "have_liblua" here to skip the tests for a system Lua library;
  # liblua_CFLAGS and liblua_LIBS will be set below
  have_liblua=yes
fi
#
# Next, try luajit using pkg-config.
#
if test $have_liblua = no; then
  PKG_CHECK_MODULES([luajit], [luajit], [have_luajit=yes], [:])
  have_liblua=$have_luajit
  liblua_CFLAGS=$luajit_CFLAGS
  liblua_LIBS=$luajit_LIBS
fi
#
# Next, try Lua using pkg-config (as present on e.g. debian).
#
if test $have_liblua = no; then
  PKG_CHECK_MODULES([liblua], [lua5.1], [have_liblua=yes], [:])
fi
#
# If we couldn't find anything using pkg-config, try searching
# manually for a Lua installation (the default lua distribution
# doesn't support pkg-config).
#
if test $have_liblua = no; then
  AC_LANG_PUSH(C)
  have_lua_h=no
  lua_include_flags=""
  AC_CHECK_HEADER([lua.h], [have_lua_h=yes])
  if test $have_lua_h = no; then
    SNOGRAY_CHECK_SUBDIR_HEADER(
       [lua_include_dir], [<lua.h>],
       [lua5.1 lua-5.1 lua51 lua],
       [have_lua_h=yes; lua_include_flags="-I$lua_include_dir"])
  fi
  if test $have_lua_h = yes; then
    for lua_lib_name in lua lua5.1 lua-5.1 lua51; do
      AC_CHECK_LIB([$lua_lib_name], [lua_close], [have_liblua=yes], [have_liblua=no], [-lm])
      if test $have_liblua = yes; then
	AC_SUBST([liblua_CFLAGS], ["$lua_include_flags"])
	AC_SUBST([liblua_LIBS], ["-l$lua_lib_name"])
	break
      fi
    done
  fi
  AC_LANG_POP(C)
fi
if test $have_liblua = no || test $build_lua_from_src = yes; then
  if test x"$lua_src" != xnone; then
    AC_SUBST([liblua_CFLAGS], ['-I$(LUA_SRC)'])
    AC_SUBST([liblua_LIBS], ['$(top_builddir)/liblua/liblua.a'])
    build_lua_from_src=yes
    have_liblua=yes
  fi
else
  build_lua_from_src=no
fi
if test "$have_liblua" = no; then
  AC_MSG_ERROR([A Lua library is required, and none was found; aborting...])
fi
if test $have_luajit = yes; then
  AC_DEFINE([HAVE_LUAJIT], [1], [Define if using LuaJIT])
fi
AM_CONDITIONAL([have_luajit], [test $have_luajit = yes])

AC_MSG_CHECKING([whether to build Lua from source])
AC_MSG_RESULT([$build_lua_from_src])
AM_CONDITIONAL([build_lua], [test $build_lua_from_src = yes])


# Check for FFTW3 optimized FFT library; we actually check for
# "fftw3f", which is the single-precision variant.
#
fftw3_precision=f
have_libfftw3=no
PKG_CHECK_MODULES([libfftw3], [fftw3$fftw3_precision], [have_libfftw3=yes], [:])

# Look for the FFTW3 threading library too.  The FFTW3 pkg-config info
# file doesn't know about the threading library, so just look for it
# the old-fashioned way...
#
use_fftw3_threads=no
if test $have_libfftw3 = yes; then
  AC_CHECK_LIB([fftw3${fftw3_precision}_threads],
	       [fftw${fftw3_precision}_init_threads],
	       [use_fftw3_threads=yes], [:], [$libfftw3_LIBS])
  if test $use_fftw3_threads = yes; then
    # prepend the threading library to the other FFTW3 libraries
    libfftw3_LIBS="-lfftw3${fftw3_precision}_threads $libfftw3_LIBS"
    AC_DEFINE([USE_FFTW3_THREADS], [1], [Define if FFTW3 multi-threading should be used])
  fi
fi

# The "snogbloom" program depends on FFTW3, so only build it when that's
# available.
#
AM_CONDITIONAL([build_snogbloom], [test $have_libfftw3 = yes])


# Check for PNG image library
#
have_libpng=no
PKG_CHECK_MODULES([libpng], [libpng], [have_libpng=yes], [:])
if test $have_libpng = yes; then
  AC_DEFINE([HAVE_LIBPNG], [1], [Define if libpng is installed])
fi
AM_CONDITIONAL([have_libpng], [test $have_libpng = yes])

# Configure some details of the PNG image library, if we have it.
#
if test $have_libpng = yes; then

  _OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CFLAGS $libpng_CFLAGS"

  AC_MSG_CHECKING([for png_set_expand_gray_1_2_4_to_8 in libpng])
  AC_TRY_COMPILE([#include <png.h>],
  		 [png_structp libpng_struct = 0;
		  png_set_expand_gray_1_2_4_to_8 (libpng_struct)],
		 [have_png_set_expand_gray_1_2_4_to_8=yes],
		 [have_png_set_expand_gray_1_2_4_to_8=no])
  AC_MSG_RESULT([$have_png_set_expand_gray_1_2_4_to_8])

  if test $have_png_set_expand_gray_1_2_4_to_8 = yes; then
    AC_DEFINE([HAVE_PNG_SET_EXPAND_GRAY_1_2_4_TO_8], [1],
	      [Define if libpng has "png_set_expand_gray_1_2_4_to_8"])
  fi

  CPPFLAGS="$_OLD_CPPFLAGS"
fi

# Check for OpenEXR image library
#
have_libexr=no
PKG_CHECK_MODULES([libexr], [OpenEXR], [have_libexr=yes], [:])
if test $have_libexr = yes; then
  AC_DEFINE([HAVE_LIBEXR], [1], [Define if libOpenEXR is installed])
fi
AM_CONDITIONAL([have_libexr], [test $have_libexr = yes])

# Check for jpeg image library
#
have_libjpeg=no
AC_CHECK_LIB([jpeg], [jpeg_start_compress], [have_libjpeg=yes])
if test $have_libjpeg = yes; then
  AC_SUBST([libjpeg_LIBS], [-ljpeg])
  AC_DEFINE([HAVE_LIBJPEG], [1], [Define if libjpeg is installed])
fi
AM_CONDITIONAL([have_libjpeg], [test $have_libjpeg = yes])

# Check for netpbm (pbm/pgm/ppm/pam) image library
#
have_libnetpbm=no
AC_CHECK_LIB([netpbm], [ppm_writeppminit], [have_libnetpbm=yes])
if test $have_libnetpbm = yes; then
  AC_SUBST([libnetpbm_LIBS], [-lnetpbm])
  AC_DEFINE([HAVE_LIBNETPBM], [1], [Define if libnetpbm is installed])
fi
AM_CONDITIONAL([have_libnetpbm], [test $have_libnetpbm = yes])

# Check for 3ds scene format library.
#
# Try to handle the case where no AM_PATH_LIB3DS autoconf macro is
# defined.
#
have_lib3ds=no
m4_define_default([AM_PATH_LIB3DS], [:])
if test $cross_compiling = no; then
  AM_PATH_LIB3DS([1.2.0], [have_lib3ds=yes])
fi
if test $have_lib3ds = yes; then
  AC_DEFINE([HAVE_LIB3DS], [1], [Define if lib3ds is installed])
fi
AM_CONDITIONAL([have_lib3ds], [test $have_lib3ds = yes])


# See if lib3ds supports the "object_flags" feature, which handles
# LIB3DS_OBJ_ chunks (in particular the LIB3DS_OBJ_HIDDEN chunk, which
# is used by many real-world models).  This feature is the upstream
# version of my own earlier "obj_flags" extension (see below).
#
if test $have_lib3ds = yes; then

  _OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CFLAGS $LIB3DS_CFLAGS"

  AC_MSG_CHECKING([whether lib3ds supports object_flags])
  AC_TRY_COMPILE([#include <lib3ds/mesh.h>],
  		 [extern Lib3dsMesh *m; m->object_flags = LIB3DS_OBJECT_HIDDEN;],
		 [have_lib3ds_object_flags=yes],
		 [have_lib3ds_object_flags=no])
  AC_MSG_RESULT([$have_lib3ds_object_flags])

  if test $have_lib3ds_object_flags = yes; then
    AC_DEFINE([HAVE_LIB3DS_OBJECT_FLAGS], [1],
	      [Define if lib3ds has "object_flags" support])
  fi

  CPPFLAGS="$_OLD_CPPFLAGS"
fi

# If "objects_flags" isn't supported, see if lib3ds supports the
# "obj_flags" extension, which adds handling of LIB3DS_OBJ_ chunks (in
# particular the LIB3DS_OBJ_HIDDEN chunk, which is used by many
# real-world models).  This is my earlier version of what was eventually
# added to the official sources as "object_flags".
#
if test $have_lib3ds = yes && test $have_lib3ds_object_flags = no; then

  _OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CFLAGS $LIB3DS_CFLAGS"

  AC_MSG_CHECKING([whether lib3ds supports obj_flags extension])
  AC_TRY_COMPILE([#include <lib3ds/mesh.h>],
  		 [extern Lib3dsMesh *m; m->obj_flags = LIB3DS_OBJF_HIDDEN;],
		 [have_lib3ds_obj_flags=yes],
		 [have_lib3ds_obj_flags=no])
  AC_MSG_RESULT([$have_lib3ds_obj_flags])

  if test $have_lib3ds_obj_flags = yes; then
    AC_DEFINE([HAVE_LIB3DS_OBJ_FLAGS], [1],
	      [Define if lib3ds has "obj_flags" support])
  fi

  CPPFLAGS="$_OLD_CPPFLAGS"
fi


# See if lib3ds supports the "LIB3DS_HIDDEN" node flag.
#
if test $have_lib3ds = yes; then

  _OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CFLAGS $LIB3DS_CFLAGS"

  AC_MSG_CHECKING([whether lib3ds supports the HIDDEN node flag])
  AC_TRY_COMPILE([#include <lib3ds/node.h>],
  		 [extern Lib3dsNode *n; n->flags1 = LIB3DS_HIDDEN;],
		 [have_lib3ds_node_hidden_flag=yes],
		 [have_lib3ds_node_hidden_flag=no])
  AC_MSG_RESULT([$have_lib3ds_node_hidden_flag])

  if test $have_lib3ds_node_hidden_flag = yes; then
    AC_DEFINE([HAVE_LIB3DS_NODE_HIDDEN_FLAG], [1],
	      [Define if lib3ds defines the node LIB3DS_HIDDEN flag])
  fi

  CPPFLAGS="$_OLD_CPPFLAGS"
fi


# See if lib3ds has "lib3ds_matrix_mult", which for whatever reason,
# _replaced_ lib3ds_matrix_mul in lib3ds 1.3.0.
#
if test $have_lib3ds = yes; then

  _OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $LIB3DS_CFLAGS"

  AC_MSG_CHECKING([for lib3ds_matrix_mult])
  AC_TRY_COMPILE([#include <lib3ds/matrix.h>],
  		 [Lib3dsMatrix N, M; lib3ds_matrix_mult (N, M);],
		 [have_lib3ds_matrix_mult=yes],
		 [have_lib3ds_matrix_mult=no])
  AC_MSG_RESULT([$have_lib3ds_matrix_mult])

  if test $have_lib3ds_matrix_mult = yes; then
    AC_DEFINE([HAVE_LIB3DS_MATRIX_MULT], [1],
	      [Define if lib3ds defines "lib3ds_matrix_mult"])
  fi

  CPPFLAGS="$_OLD_CPPFLAGS"
fi


# Check for unique_ptr (the c++11 version of auto_ptr)
#
AC_MSG_CHECKING([for std::unique_ptr])
AC_LINK_IFELSE(
  [AC_LANG_SOURCE(
     [#include <memory>
      int main () { int x; std::unique_ptr<int> y (&x); return 0; }])],
  [have_std_unique_ptr=yes],
  [have_std_unique_ptr=no])
AC_MSG_RESULT([$have_std_unique_ptr])
if test $have_std_unique_ptr = yes; then
  AC_DEFINE([HAVE_STD_UNIQUE_PTR], [1],
    [Define if std::unique_ptr is supported])
fi


##
## Threading
##


# Check for standard C++ mutex support
#
AC_MSG_CHECKING([for std::mutex])
AC_LINK_IFELSE(
  [AC_LANG_SOURCE(
     [#include <mutex>
      int main () { std::mutex m; m.lock(); m.unlock (); }])],
  [have_std_mutex=yes],
  [have_std_mutex=no])
AC_MSG_RESULT([$have_std_mutex])


# Check for standard C++ thread support
#
AC_MSG_CHECKING([for std::thread])
AC_LINK_IFELSE(
  [AC_LANG_SOURCE(
     [#include <thread>
      void f () { }
      int main () { std::thread t (f); }])],
  [have_std_thread=yes],
  [have_std_thread=no])
AC_MSG_RESULT([$have_std_thread])


# Check for the boost thread library
#
have_boost_thread=no
AC_CHECK_HEADERS([boost/thread.hpp])
if test "$ac_cv_header_boost_thread_hpp" = yes; then
  _OLD_LIBS="$LIBS"
  boost_thread_lib="-lboost_thread-mt"
  LIBS="$LIBS $boost_thread_lib"
  AC_MSG_CHECKING([for -lboost_thread-mt])
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE(
       [#include <boost/thread.hpp>
        int main () { boost::this_thread::yield (); return 0; }])],
    [have_lib_boost_thread_mt=yes],
    [have_lib_boost_thread_mt=no])
  AC_MSG_RESULT([$have_lib_boost_thread_mt])
  have_boost_thread=$have_lib_boost_thread_mt
  LIBS="$_OLD_LIBS"
fi


# Choose which thread interface to use
#
AC_MSG_CHECKING([which thread interface to use])
if test "$have_std_thread,$have_std_mutex" = "yes,yes"; then
  AC_DEFINE([USE_STD_THREAD], [1], [Define if using c++ std thread interface])
  thread_api_name="std::thread (c++11)"
elif test $have_boost_thread = yes; then
  AC_DEFINE([USE_BOOST_THREAD], [1], [Define if using boost thread library])
  LIBS="$LIBS $boost_thread_lib"
  thread_api_name="boost::thread"
else
  thread_api_name=none
fi
AC_MSG_RESULT([$thread_api_name])

# A config variable that says whether any threading is available
#
if test "$thread_api_name" != none; then
  AC_DEFINE([USE_THREADS], [1], [Define if we can use multiple threads])
fi
AM_CONDITIONAL([use_threads], [test "$thread_api_name" != none])


##
## Random-number generators
##


# Check for C++11 std::random
#
have_std_random=no
AC_MSG_CHECKING([for c++11 std::random])
AC_LINK_IFELSE(
  [AC_LANG_SOURCE(
     [#include <random>
      std::mt19937 rng;
      std::uniform_real_distribution<float> dist;
      int main () { dist (rng); }])],
  [have_std_random=yes])
AC_MSG_RESULT([$have_std_random])

# Check for C++0x TR1 std::random, but only if we didn't get the later version
#
have_std_tr1_random=no
if test "$have_std_random" = no; then
  AC_MSG_CHECKING([for c++0x TR1 std::random])
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE(
       [#include <random>
	std::mt19937 rng;
	std::uniform_real<float> dist;
	std::variate_generator<std::mt19937 &, std::uniform_real<float> > vg (rng, dist);
	int main () { vg (); }])],
    [have_std_tr1_random=yes])
  AC_MSG_RESULT([$have_std_tr1_random])
fi

# Check for the boost random library
#
have_boost_random=no
AC_CHECK_HEADERS([boost/random.hpp])
if test "$ac_cv_header_boost_random_hpp" = yes; then
  AC_MSG_CHECKING([for boost::random])
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE(
       [#include <boost/random.hpp>
	boost::mt19937 rng;
	boost::uniform_real<float> dist;
	boost::variate_generator<boost::mt19937 &, boost::uniform_real<float> > vg (rng, dist);
	int main () { vg (); }])],
    [have_boost_random=yes])
  AC_MSG_RESULT([$have_boost_random])
fi


# Choose which random number generator to use
#
AC_MSG_CHECKING([which random number generator to use])
if test "$have_std_random" = "yes"; then
  AC_DEFINE([USE_STD_RANDOM], [1], [Define if using c++11 std::random])
  random_api_name="std::random (c++11)"
elif test $have_boost_random = yes; then
  AC_DEFINE([USE_BOOST_RANDOM], [1], [Define if using boost::random])
  random_api_name="boost::random"
elif test "$have_std_tr1_random" = "yes"; then
  AC_DEFINE([USE_STD_TR1_RANDOM], [1], [Define if using c++0x TR1 std::random])
  random_api_name="std::random (c++0x TR1)"
else
  random_api_name="rand"
fi
AC_MSG_RESULT([$random_api_name])


##
## ----------------------------------------------------------------
## Some configuration options
##

# Whether we have "swig", which is the interface generator we use to
# generate the Lua-to-C++ interface.
#
have_swig=no
test -n "$SWIG" && have_swig=yes
AM_CONDITIONAL([have_swig], [test $have_swig = yes])

# Whether we have a pre-generated Lua-C++ interface
# "snograw_lua_wrap.cc", or have the ability to generate it.
#
# "snograw_lua_wrap.cc" is normally generated from "snograw.swg" etc,
# by running the "swig" interface generator, but if we don't have
# swig, we can still use any existing copy of that files (however any
# changes in snograw.swg made by the user etc won't be reflected).
#
have_snograw_lua_wrap=$have_swig
test -r "snograw_lua_wrap.cc" && have_snograw_lua_wrap=yes

if test $have_snograw_lua_wrap = no; then
  AC_MSG_ERROR([either SWIG or a pre-generated snograw_lua_wrap.cc file is required, and neither was found; aborting...])
fi

# Whether we can use the swig "disown" feature to transfer object
# ownership from Lua to C++.  This is only implemented for Lua starting
# with swig version 1.3.35.
#
if test $have_swig = yes; then
  AC_MSG_CHECKING([whether swig supports "disown" feature])

  cat > conftest.swg <<EOF
%module test;
%apply SWIGTYPE* DISOWN {X *}
EOF
  if $SWIG 2>/dev/null -Werror -o /dev/null -c++ -lua conftest.swg; then
    have_swig_disown=yes
  else
    have_swig_disown=no
  fi
  rm -f conftest.swg

  AC_MSG_RESULT([$have_swig_disown])

  if test "$have_swig_disown" = no; then
    AC_MSG_ERROR([SWIG version 1.3.35 or later (supporting the "disown" feature) is required; aborting...])
  fi
fi

# See whether we can use the swig -nomoduleglobal option.
#
if test $have_swig = yes; then
    AC_MSG_CHECKING([whether swig supports "-nomoduleglobal" option])

    cat > conftest.swg <<EOF
%module test;
EOF
    if $SWIG 2>/dev/null -o /dev/null -c++ -lua -nomoduleglobal conftest.swg; then
      have_swig_nomoduleglobal=yes
    else
      have_swig_nomoduleglobal=no
    fi
    rm -f conftest.swg

    AC_MSG_RESULT([$have_swig_nomoduleglobal])

    if test "$have_swig_nomoduleglobal" = yes; then
      AC_SUBST([SWIG_NOMODULEGLOBAL_FLAG], [-nomoduleglobal])
    fi
fi


##
## ----------------------------------------------------------------
## Try to set compiler flags appropriately
##

# Various flags get added to these, and then validated below.
#
# "REQ" flags are always required, even when not optimizing (e.g. for
# correctness); "OPT" flags are optimization flags.
#
CXX_EXTRA_REQ_FLAGS=""
CXX_EXTRA_OPT_FLAGS=""


if test "$GXX" = yes; then

  # These flags should always be accepted by g++
  #
  # The following settings are based on g++ 4.4, compiling for the Intel
  # core2.
  #
  # -fomit-frame-pointer seems to increase code size appreciably when
  # used with -Os, so we omit it in that case.
  #
  AC_SUBST([CXX_OPT_FLAGS], ["-O3 -fomit-frame-pointer"])
  AC_SUBST([CXX_OPT_SIZE_FLAGS], ["-Os"])

  # If link-time optimization is enabled, try the gcc option for that.
  #
  # Note that we intentionally only add "-flto" to the default
  # optimization flags (CXX_OPT_FLAGS), _not_ the size-optimization
  # floats (CXX_OPT_SIZE_FLAGS).
  #
  if test "$lto" = yes; then
    # See if the compiler even accepts the option
    #
    SNOGRAY_CHECK_CXX_FLAGS([CXX_LTO_FLAGS], [-flto])

    # If the compiler was OK with it, try a test compile to see if it
    # actually seems to work; we need to test all the way through
    # linking, because, e.g. with clang, the compiler succeeds, but the
    # linker can't handle what it outputs.
    #
    if test x"$CXX_LTO_FLAGS" != x; then
      _OLD_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="$CXX_LTO_FLAGS $CXXFLAGS"
      AC_MSG_CHECKING([whether compiler "$CXX_LTO_FLAGS" option is broken])
      AC_LINK_IFELSE(
	[AC_LANG_SOURCE([int main () { return 0; }])],
	[cxx_lto_broken=no],
        [cxx_lto_broken=yes])
      AC_MSG_RESULT([$cxx_lto_broken])
      CXXFLAGS="$_OLD_CXXFLAGS"

      if test $cxx_lto_broken = no; then
	CXX_OPT_FLAGS="$CXX_OPT_FLAGS $CXX_LTO_FLAGS"
      fi
    fi
  fi

  # g++ flags for architecture-independent floating-point optimization.
  #
  # -ffast-math turns on -ffinite-math-only by default, but in rare cases,
  # it's possible for infinity to be generated, so we use
  # -fno-finite-math-only to override that.
  #
  # When given the -ffast-math option, g++ will by default speculatively
  # evaluate floating-point expressions even if they may trap (e.g., divide).
  # The -ftrapping-math option causes g++ to be more careful in such cases,
  # so use that if we're going to enable floating-point exceptions.
  #
  # Currently, -ftrapping-math causes g++ to automatically disable
  # -fassociative-math, but prints an annoying warning in the process.
  # To avoid this, we check for the warning, and if found, explicitly
  # use -fno-associative-math to silence it.
  #
  CXX_MATH_OPT_FLAGS="-ffast-math"
  CXX_MATH_REQ_FLAGS="-fno-finite-math-only"   # we allow infinity
  if test "$use_fp_exceptions" = yes; then
    CXX_MATH_REQ_FLAGS="$CXX_MATH_REQ_FLAGS -ftrapping-math"

    # Check for "-fassociative-math disabled" warning, and if found,
    # explicitly use -fno-associative-math to silence it.
    #
    assoc_math_disabled_warning=yes
    AC_MSG_CHECKING([for "-fassociative-math disabled" warning])
    AC_EGREP_CPP([fassociative-math disabled], [], [assoc_math_disabled_warning=no])
    AC_MSG_RESULT([$assoc_math_disabled_warning])
    if test "$assoc_math_disabled_warning" = yes; then
      CXX_MATH_REQ_FLAGS="$CXX_MATH_REQ_FLAGS -fno-associative-math"
    fi
  fi

  # Add extra optimization flags.
  #
  CXX_EXTRA_OPT_FLAGS="$CXX_EXTRA_OPT_FLAGS $CXX_MATH_OPT_FLAGS"
  CXX_EXTRA_REQ_FLAGS="$CXX_EXTRA_REQ_FLAGS $CXX_MATH_REQ_FLAGS"


  # First try the "-march=native" option
  #
  SNOGRAY_CHECK_CXX_FLAG([CXX_MACH_FLAGS], [-march=native])

  # If -march=native seemed to work, test for a gcc bug that makes using
  # it a bad idea.
  #
  # Some versions of gcc, on some platforms, will accept -march=native,
  # but don't set the compiler tuning parameters according to the
  # selected native architecture.  This results in really crappy code,
  # so it's better just not to use -march=native at all in such cases.
  #
  if test x"$CXX_MACH_FLAGS" != x && test "$GCC" = yes; then

    # Gcc defines some predefined macros reflecting compiler tuning
    # choices.  In the buggy case described above, there are none, so we
    # basically just look for _any_ defined macros matching the pattern
    # "__tune_".  To do this, however, we use the gcc "-dD" option,
    # which outputs a list of all defined macros.  We also explicitly
    # add -march=native to CPPFLAGS (normally it's added to CFLAGS or
    # CXXFLAGS instead).

    _OLD_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -dD -march=native"

    buggy_march_native=yes
    AC_MSG_CHECKING([whether gcc -march=native is buggy])
    AC_EGREP_CPP([__tune_], [], [buggy_march_native=no])
    AC_MSG_RESULT([$buggy_march_native])

    CPPFLAGS="$_OLD_CPPFLAGS"

    # If we found the bug, just don't use -march=native
    #
    if test $buggy_march_native = yes; then
      CXX_MACH_FLAGS=""
    fi
  fi

  # If -march=native worked (and isn't buggy), then just use that.
  #
  if test x"$CXX_MACH_FLAGS" != x; then
    #
    # It did, so try to turn on -mfpmath=sse, which is generally better
    # for machines that support it.

    SNOGRAY_CHECK_CXX_FLAG([CXX_MACH_FLAGS], [$CXX_MACH_FLAGS -mfpmath=sse])
  else
    #
    # -march=native didn't work, so try to set machine-specific options
    # based on what we think the cpu type is.

    # We try to use the host type to set optimization flags
    #
    AC_CANONICAL_HOST

    # If this is a linux system with /proc/cpuinfo we can be a bit more specific
    #
    if test -r /proc/cpuinfo; then
      AC_MSG_CHECKING([CPU type in /proc/cpuinfo])

      cpuinfo_cpu=unknown
      cpuinfo_note=''

      [case "$host_cpu" in
	i?86|x86[-_]64)
	  cpuinfo=`$AWK '
	    /^cpu family[ \t]*:/ { family = $NF; next }
	    /^model[ \t]*:/ { model = $NF; next }
	    /^model name[ \t]*:/ {
	      sub (/^.*: */, "")
	      gsub (/ *\([^)]*\)/, "")
	      gsub (/[ \t]+/, " ")
	      sub (/ *@ *[0-9.]+[GM][Hh][Zz] *$/, "")
	      name = $0
	      next
	    }
	    END { print family "/" model "/" name }
	  ' /proc/cpuinfo`

	  cpuinfo_name=`basename "$cpuinfo"`
	  cpuinfo_note=" ($cpuinfo_name)"

	  case "$cpuinfo" in
	    *"Pentium III"*)
	      cpuinfo_cpu=pentium3 ;;
	    *"Pentium 4"*)
	      cpuinfo_cpu=pentium4 ;;
	    *"Pentium M"*)
	      cpuinfo_cpu=pentium-m ;;
	    *"Opteron"*)
	      cpuinfo_cpu=opteron ;;
	    *"Core2"*)
	      cpuinfo_cpu=core2 ;;
	    *"Phenom"*)
	      cpuinfo_cpu=amdfam10 ;;
	    15/65/*)
	      cpuinfo_cpu=opteron ;;
	    15/*)
	      cpuinfo_cpu=pentium4 ;;
	    16/*)
	      cpuinfo_cpu=amdfam10 ;;
	    6/9/*)
	      cpuinfo_cpu=pentium4 ;;
            6/23/*)
	      cpuinfo_cpu=core2 ;;
	  esac
	  ;;
      esac]

      AC_MSG_RESULT([$cpuinfo_cpu$cpuinfo_note])

      test "$cpuinfo_cpu" = unknown || host_cpu="$cpuinfo_cpu"
    fi

    archs=""
    extra_arch_opts=""
    case "$host_cpu" in
      i686|pentium3)
	archs="pentium3"
	extra_arch_opts="-mfpmath=sse" ;;
      i786|pentium4)
	archs="pentium4"
	extra_arch_opts="-mfpmath=sse" ;;
      pentium-m)
	archs="pentium-m pentium3"
	extra_arch_opts="-mfpmath=sse" ;;
      x86[-_]64)
	archs="k8 athlon"
	extra_arch_opts="-mfpmath=sse" ;;
      amdfam10)
	archs="amdfam10 opteron k8 athlon"
	extra_arch_opts="-mfpmath=sse" ;;
      core2)
	archs="core2 pentium-m pentium3"
	extra_arch_opts="-mfpmath=sse" ;;
      opteron)
	archs="opteron athlon"
	extra_arch_opts="-mfpmath=sse" ;;
      athlon)
	archs="athlon"
	extra_arch_opts="-mfpmath=sse" ;;
    esac

    # Try to find an architecture in $archs that the compiler accepts
    #
    for arch in $archs; do

      # See if the compiler accepts $arch as an architecture.
      #
      SNOGRAY_CHECK_CXX_FLAG([CXX_MARCH], [-march=$arch])

      # If so, use it, possibly supplemented by $extra_arch_opts.
      #
      if test x"$CXX_MARCH" != x; then

	# Check machine options for validity; note that we check them all
	# together, not individually, as some may depend on others.
	#
	if test x"$extra_arch_opts" != x; then
	  SNOGRAY_CHECK_CXX_FLAG([CXX_MACH_FLAGS], [$CXX_MARCH $extra_arch_opts])
	fi

	# If the above didn't define CXX_MACH_FLAGS, just use CXX_MARCH alone
	#
	if test x"$CXX_MACH_FLAGS" = x; then
	  CXX_MACH_FLAGS="$CXX_MARCH"
	fi

	break
      fi
    done
  fi

else

  AC_MSG_WARN([Not using GNU compiler, so not setting optimization flags])

  # This is accepted by almost every compiler.
  #
  AC_SUBST([CXX_OPT_FLAGS], [-O])

fi


# Test for the "-fuse-ld=gold" option to explicitly select the GNU
# gold linker (rather than the default GNU binutils "BFD" linker).
# Gold is a better linker than the default binutils linkers, so we'd
# like to use it if possible.
#
# Note that this check should come before the check for "--icf=all",
# as the latter only works with gold.
#
SNOGRAY_CHECK_CXX_LINK_FLAG([CXX_GOLD_LDFLAGS], [-fuse-ld=gold])
if test x"$CXX_GOLD_LDFLAGS" != x; then
  CONFIG_LDFLAGS="$CONFIG_LDFLAGS $CXX_GOLD_LDFLAGS"
  LDFLAGS="$LDFLAGS $CXX_GOLD_LDFLAGS"
fi

# Test for the GNU gold linker's "--icf=all" option, which folds
# duplicate sections of code, using the gcc linker-flag prefix "-Wl,".
#
# "--icf=all" folds all duplicate sections.  "--icf=safe" is a
# somewhat less aggressive variant, which uses heuristics to avoid
# folding potentially unsafe cases (where function pointers are used
# in non-vtable context); however our code should all be safe for
# --icf=all (the real question is libraries ...).
#
SNOGRAY_CHECK_CXX_LINK_FLAG([CXX_ICF_LDFLAGS], [-Wl,--icf=all])

# If "-Wl,--icf=all" is supported, also check for the compiler
# "--ffunction-sections" option; this puts each function into its own
# separate linker section, which increases the opportunities for --icf
# to work.
#
if test x"$CXX_ICF_LDFLAGS" != x; then
  CONFIG_LDFLAGS="$CONFIG_LDFLAGS $CXX_ICF_LDFLAGS"
  CXX_EXTRA_REQ_FLAGS="$CXX_EXTRA_REQ_FLAGS -ffunction-sections"
fi

# gcc requires the -pthread flag when using multi-threading (without
# -pthread, threading primitives will still _kinda_ work, with weird
# and hard-to-debug problems, like mutexes silently doing nothing...),
# so add that if necessary.
#
# If other compilers have some a similar requirement for other
# options, they could be added here.
#
if test "$thread_api_name" != none; then
  CXX_EXTRA_REQ_FLAGS="$CXX_EXTRA_REQ_FLAGS -pthread"
fi


# Validate whatever's in CXX_EXTRA_REQ_FLAGS and CXX_EXTRA_OPT_FLAGS.
# The resulting subst in the Makefile will only contain those the
# compiler actually accepts.
#
SNOGRAY_CHECK_CXX_FLAGS([CXX_EXTRA_REQ_FLAGS], [$CXX_EXTRA_REQ_FLAGS])
SNOGRAY_CHECK_CXX_FLAGS([CXX_EXTRA_OPT_FLAGS], [$CXX_EXTRA_OPT_FLAGS])


# This variable can be set by the user to pass in extra compiler
# options, in addition to optimization and debug options (which are
# usually set by other means).
#
AC_SUBST([EXTRA_COMPILE_FLAGS], [])


##
## ----------------------------------------------------------------
## Final output
##

# Reset various compiler-option variables to whatever the user
# specified.
#
CFLAGS=$USER_CFLAGS
CXXFLAGS=$USER_CXXFLAGS
CPPFLAGS=$USER_CPPFLAGS
LDFLAGS=$USER_LDFLAGS


AC_CONFIG_FILES([Makefile camera/Makefile cli/Makefile color/Makefile
			  doc/Makefile geometry/Makefile
			  glare/Makefile image/Makefile light/Makefile
			  liblpeg/Makefile liblua/Makefile
			  load/Makefile lua/Makefile lua-util/Makefile
			  material/Makefile photon/Makefile
			  render/Makefile render-mgr/Makefile
			  snograw/Makefile space/Makefile
			  surface/Makefile texture/Makefile
			  util/Makefile])

AC_OUTPUT


m4_if(dnl	Do not change this comment
   arch-tag: 336310fb-78ae-4d49-94a3-d6ede54ce9dd
)dnl
