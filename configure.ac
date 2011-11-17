AC_INIT([snogray], [0.4], [Miles Bader <miles@gnu.org>])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_HEADERS([config.h])


AC_PROG_CC
AC_PROG_CXX

AC_PROG_RANLIB

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


# Enable/disable single-precision floating-point coordinates
#
# Using single-precision coordinates is much faster, but can present
# problems with precision errors (typically, though, this can be worked
# around at runtime by increasing the value of the "min-trace" rendering
# parameter).
#
AC_ARG_ENABLE([float-coords],
    AS_HELP_STRING([--disable-float-coords],
		   [Use double-precision coordinates]),
    [enable_float_coords="$enableval"],
    [enable_float_coords=yes])
AC_MSG_CHECKING([whether to use single-precision coordinates])
AC_MSG_RESULT([$enable_float_coords])
if test "$enable_float_coords" = yes; then
  AC_DEFINE([USE_FLOAT_COORDS], [1],
  	    [Define if using single-precision coordinates])
fi


##
## --------------------------------
## Optional addons
##


# --with-lua-src=DIR option

lua_src_specified=no
AC_ARG_WITH([lua-src],
    AS_HELP_STRING([--with-lua-src], [Specify Lua source directory]),
    [lua_src_specified=yes], [with_lua_src=$srcdir/lua])

AC_MSG_CHECKING([for Lua source directory])
lua_dist=`echo "$with_lua_src" | sed 's@^./@@'`
lua_src=none
for dir in "$lua_dist" "$lua_dist/src"; do
    if test -r "$dir/lapi.c"; then
       lua_src="$dir"
       break
    fi
done
AC_MSG_RESULT([$lua_src])

build_lua_in_src_dir=no
if test "$lua_src" != none; then
  AC_SUBST([LUA_DIST], [$lua_dist])
  AC_SUBST([LUA_SRC], [$lua_src])

  # We use two different methods for building liblua.a, depending on whether
  # we're using a separate build directory or not, and whether the Lua source
  # directory is a subdirectory (and thus is "ours"):
  #
  # + If building in the source dir, and the Lua source directory is a simple
  #   subdirectory of the top-level source dir, we build liblua.a in the Lua
  #   source directory and copy it to our top-level directory.  This avoids
  #   most assumptions about how make works.
  #
  # + If using a separate build dir, or if the Lua source directory is
  #   "somewhere else", we build liblua.a in the current directory, using the
  #   Makefile from the Lua source directory, and VPATH to tell make where to
  #   find the source files. This requires that make support at using VPATH
  #   to find source files, which seems a reasonable assumption in this case.
  #   It also requires that no lua object files have the same name as any of
  #   our object files; that's the case currently (all Lua source/object
  #   files begin with "l").
  #
  # Our test to determine if LUA_SRC is "our" subdirectory is very simple, we
  # just see if $lua_dist _doesn't_ contain a slash (we use $lua_dist instead
  # of $lua_src because $lua_src is often a subdirectory of $lua_dist, and so
  # will have a slash added).
  #
  AC_MSG_CHECKING([whether to build Lua in source directory])
  lua_dist_slash=`echo "$lua_dist" | grep /`
  if test x"$srcdir" = x. && test x"$lua_dist_slash" != x"$lua_dist"; then
    build_lua_in_src_dir=yes
  fi
  AC_MSG_RESULT([$build_lua_in_src_dir])
fi

AM_CONDITIONAL([build_lua], [test x"$lua_src" != xnone])

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
for opt in -std=c++11 -std=c++0x -std=c++98; do
  SNOGRAY_CHECK_CXX_FLAG([CXX_STD_FLAGS], [$opt])

  case "$CXX_STD_FLAGS" in
    "-std=c++0x")
      # Some compilers (versions of g++ 4.3, clang with a mismatched
      # version of libstdc++) have broken support for c++0x, so try to
      # detect problematic cases we know about, and revert to c++98 if
      # compilation fails.
      #
      # This is just a mismash of code containing valid bits of C++0x
      # code that fail to compile on various compilers.
      #
      AC_MSG_CHECKING([checking whether c++0x support is broken])
      _OLD_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="$CXXFLAGS $CXX_STD_FLAGS"
      AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE(
	    [#include <iostream>
	     #include <type_traits>
	     bool file_exists () { return true; }
	     using namespace std;
	     bool test () { return file_exists (); }])],
	[broken_cxx0x=no],
	[broken_cxx0x=yes])
      CXXFLAGS="$_OLD_CXXFLAGS"
      AC_MSG_RESULT([$broken_cxx0x])
      if test "$broken_cxx0x" = yes; then
	CXX_STD_FLAGS=""
	cxx_level_note=" (c++0x support broken)"
      fi
      ;;
  esac

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
  [-Wall -Wextra -Winit-self -pedantic-errors -Wno-long-long])


##
## --------------------------------
## Language feature tests
##

#
# See if this compiler supports the "extern template class Foo<T>"
# syntax for suppressing template instantiation of specific types.
# This is availble in C++0x, and also as a g++ extension to C++98.
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
  # There are two subtleties:
  # (1) We test with $CXX_STD_FLAGS and $CXX_CHECK_FLAGS added to the
  #     compiler flags (which is what will be used later for actual
  #     program compilation), as "extern template" may not be
  #     supported when they are used.
  # (2) If it doesn't seem to work at first, we try again, this time
  #     prefixing "extern template" with "__extension__"; this tells
  #     g++ to allow the feature even in strict compatibility mode.
  #
  for extension_kw in '' '__extension__'; do
    msg_suffix=""
    test x$extension_kw = x__extension__ && msg_suffix=" with $extension_kw"
    AC_MSG_CHECKING([whether C++ compiler supports "extern template"$msg_suffix])
    _OLD_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXX_STD_FLAGS $CXX_CHECK_FLAGS $CXXFLAGS"
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE([template<class T> class Foo { void bar () {} };
	 $extension_kw extern template class Foo<int>;])],
      [have_extern_template=yes],
      [have_extern_template=no])
    CXXFLAGS="$_OLD_CXXFLAGS"
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


##
## --------------------------------
## Standard library/environment tests
##


AC_CHECK_HEADERS([stdint.h unistd.h fcntl.h sys/mman.h sys/stat.h])

AC_TYPE_INTPTR_T


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
if test x"$lua_src" != xnone; then
  have_liblua=yes
  AC_SUBST([liblua_CFLAGS], ['-I$(LUA_SRC)'])
  AC_SUBST([liblua_LIBS], [liblua.a])
fi
#
# Next, try using pkg-config (as present on e.g. debian).
#
if test $have_liblua = no; then
  PKG_CHECK_MODULES([liblua], [lua5.1], [have_liblua=yes], [:])
fi
#
# If we couldn't find it using pkg-config, try without (the default lua
# distribution doesn't support pkg-config).
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
if test $have_liblua = yes; then
  AC_DEFINE([HAVE_LIBLUA], [1], [Define if liblua is installed])

  # Now see if there's a luajit library.  If there is, link against it
  # instead of the standard Lua library; the header files, are still
  # those from the normal Lua installation, and luajit is basically a
  # drop-in replacement for standard Lua, so we dont need to do much
  # else.
  #
  have_libluajit=no
  luajit_lib_name="luajit-5.1"
  AC_CHECK_LIB([$luajit_lib_name], [lua_close], [have_libluajit=yes], [:], [-lm])
  if test $have_libluajit = yes; then
    AC_SUBST([liblua_LIBS], ["-l$luajit_lib_name"])
  fi
fi
AM_CONDITIONAL([have_liblua], [test $have_liblua = yes])

# Check for PNG image library
#
have_libpng=no
PKG_CHECK_MODULES([libpng], [libpng], [have_libpng=yes], [:])
if test $have_libpng = yes; then
  AC_DEFINE([HAVE_LIBPNG], [1], [Define if libpng is installed])
fi
AM_CONDITIONAL([have_libpng], [test $have_libpng = yes])

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
m4_define_default([AM_PATH_LIB3DS], [])
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


# Check for unique_ptr (the c++0x version of auto_ptr)
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
  thread_api_name="std::thread (c++0x)"
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


# Check for C++0x std::random
#
have_std_random=no
AC_MSG_CHECKING([for c++0x std::random])
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
  AC_DEFINE([USE_STD_RANDOM], [1], [Define if using c++0x std::random])
  random_api_name="std::random (c++0x)"
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
# "snograw_lua_wrap.cc" is normally generated from "snograw.i" by
# running the "swig" interface generator, and "swigluarun.h" is also
# generated by swig (with no input), but if we don't have swig, we can
# still use any existing copies of those files (however changes in
# snograw.i won't be used).
#
have_snograw_lua_wrap=$have_swig
test -r "snograw_lua_wrap.cc" && test -r "swigluarun.h" && have_snograw_lua_wrap=yes

# Whether we can use Lua.
#
AC_MSG_CHECKING([whether to use Lua])
use_lua=no
use_lua_pretty=no
if test $have_snograw_lua_wrap = no; then
  use_lua_pretty='no (swig not installed)'
elif test "$have_liblua" = no; then
  use_lua_pretty='no (no Lua library)'
else
  use_lua=yes

  if test "$have_swig" = yes; then
    use_lua_pretty=yes
  else
    use_lua_pretty='yes (using pre-generated interface)'
  fi

  AC_DEFINE([USE_LUA], [1], [Define if lua scene definition is enabled])
fi
AC_MSG_RESULT([$use_lua_pretty])
AM_CONDITIONAL([use_lua], [test $use_lua = yes])


# Whether we can use the swig "disown" feature to transfer object
# ownership from Lua to C++.  This is only implemented for Lua starting
# with swig version 1.3.35.
#
if test $use_lua = yes; then
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

    if test "$have_swig_disown" = yes; then
      AC_DEFINE([HAVE_SWIG_DISOWN], [1], [Define if swig supports "disown" feature for Lua])
    fi
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
## Try to set optimization flags based on the type of CPU being used
##

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

  # Validate extra optimization flags.
  #
  CXX_EXTRA_OPT_FLAGS="$CXX_MATH_OPT_FLAGS"
  SNOGRAY_CHECK_CXX_FLAGS([CXX_EXTRA_OPT_FLAGS], [$CXX_EXTRA_OPT_FLAGS])

  CXX_EXTRA_REQ_FLAGS="$CXX_MATH_REQ_FLAGS"
  SNOGRAY_CHECK_CXX_FLAGS([CXX_EXTRA_REQ_FLAGS], [$CXX_EXTRA_REQ_FLAGS])


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


# Test for the GNU gold linker's "--icf=all" option, which folds
# duplicate sections of code, using the gcc linker-flag prefix "-Wl,".
#
# "--icf=all" folds all duplicate sections.  "--icf=safe" is a
# somewhat less aggressive variant, which uses heuristics to avoid
# folding potentially unsafe cases (where function pointers are used
# in non-vtable context); however our code should all be safe for
# --icf=all (the real question is libraries ...).
#
SNOGRAY_CHECK_CXX_LINK_FLAG([CXX_LDFLAGS], [-Wl,--icf=all])

# If "-Wl,--icf=all" is supported, also check for the compiler
# "--ffunction-sections" option; this puts each function into its own
# separate linker section, which increases the opportunities for --icf
# to work.
#
if test x"$CXX_LDFLAGS" != x; then
  SNOGRAY_CHECK_CXX_FLAG([CXX_FUNCTION_SECS], [-ffunction-sections])

  # We've already defined CXX_EXTRA_REQ_FLAGS above, so append to it.
  #
  CXX_EXTRA_REQ_FLAGS="$CXX_EXTRA_REQ_FLAGS $CXX_FUNCTION_SECS"
fi


# Set linker flags.
#
LDFLAGS="$CXX_LDFLAGS"


# This variable can be set by the user to pass in extra compiler
# options, in addition to optimization and debug options (which are
# usually set by other means).
#
AC_SUBST([EXTRA_COMPILE_FLAGS], [])


##
## ----------------------------------------------------------------
## Final output
##


AC_CONFIG_FILES([Makefile])

AC_OUTPUT


m4_if(dnl	Do not change this comment
   arch-tag: 336310fb-78ae-4d49-94a3-d6ede54ce9dd
)dnl
