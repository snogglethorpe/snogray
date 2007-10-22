AC_INIT([snogray], [0.1], [Miles Bader <miles@gnu.org>])

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])


AC_PROG_CC
AC_PROG_CXX

AC_PROG_RANLIB

AC_CHECK_PROG([SWIG], [swig], [swig])

# We use C++ for everything, so do all our tests using that too.
#
AC_LANG(C++)


# Enable/disable single-precision floating-point coordinates
#
# Using single-precision coordinates is much faster, but can present
# problems with precision errors (typically, though, this can be worked
# around at runtime by increasing the value of the "min-trace" rendering
# parameter).
#
AC_ARG_ENABLE([float-coords],
    AS_HELP_STRING([--enable-float-coords],
		   [Enable single-precision coordinates]),
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
## Language feature tests
##


# See if this compiler supports the "extern template class Foo<T>"
# syntax for suppressing template instantiation of specific types
# (currently this is a g++ extension).

AC_ARG_ENABLE([extern-template],
    AS_HELP_STRING([--enable-extern-template],
		   [Use C++ "extern template" extension]),
    [enable_extern_template="$enableval"],
    [enable_extern_template=yes])

if test "$enable_extern_template" = yes; then
  AC_MSG_CHECKING([whether C++ compiler supports "extern template"])
  AC_COMPILE_IFELSE(
      AC_LANG_SOURCE([template<class T> class Foo { void bar () {} };
		      extern template class Foo<int>;]),
      [have_extern_template=yes],
      [have_extern_template=no])
  AC_MSG_RESULT([$have_extern_template])
else
  have_extern_template="no"
fi

if test $have_extern_template = yes; then
  AC_DEFINE([HAVE_EXTERN_TEMPLATE], [1],
	    [Define if C++ compiler supports "extern template" extension])
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
lua_src=none
for dir in "$with_lua_src" "$with_lua_src/src"; do
    if test -r "$dir/lapi.c"; then
       lua_src="$dir"
       break
    fi
done
AC_MSG_RESULT([$lua_src])

if test "$lua_src" != none; then
  AC_SUBST([LUA_DIST], [$with_lua_src])
  AC_SUBST([LUA_SRC], [$lua_src])
fi
AM_CONDITIONAL([build_lua], [test x"$lua_src" != xnone])


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
      AC_COMPILE_IFELSE(AC_LANG_SOURCE([]), [opt_ok=yes], [opt_ok=no])
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
    CXXFLAGS="$CXXFLAGS $2"
    case "$2" in *' '*) S=s;; *) S="";; esac
    AC_MSG_CHECKING([whether C++ compiler accepts "$2" option$S])
    AC_COMPILE_IFELSE(AC_LANG_SOURCE([]), [opt_ok=yes], [opt_ok=no])
    AC_MSG_RESULT([$opt_ok])
    if test "$opt_ok" = yes; then
      AC_SUBST([$1], ["$2"])
    fi
    CXXFLAGS="$_SAVED_CXXFLAGS"
])

# Try to be very picky, if the compiler lets us.
#
SNOGRAY_CHECK_CXX_FLAGS([CXX_CHECK_FLAGS], [-Wall -Wextra -std=c++98])


##
## --------------------------------
## Standard library/environment tests
##


AC_CHECK_HEADERS([stdint.h])


# Allow point exceptions to be enabled (which can make debugging much easier).

AC_ARG_ENABLE([fp-exceptions],
    AS_HELP_STRING([--enable-fp-exceptions],
		   [Enable floating-point exceptions]),
    [enable_fp_exceptions="$enableval"],
    [enable_fp_exceptions=yes])

use_fp_exceptions=no

if test "$enable_fp_exceptions" = yes; then

  AC_CHECK_HEADERS([fenv.h])

  have_feenableexcept=no
  AC_CHECK_LIB([m], [feenableexcept], [have_feenableexcept=yes])
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
AC_MSG_RESULT([$enable_fp_exceptions])
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
   $4])


##
## --------------------------------
## External library tests
##


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
  PKG_CHECK_MODULES([liblua], [lua5.1],
    [have_liblua=yes],
    [have_liblua=no
     AC_MSG_RESULT([no])])
fi
#
# If we couldn't find it using pkg-config, try without (the default lua
# distribution doesn't support pkg-config).
#
if test $have_liblua = no; then
   AC_LANG_PUSH(C)
   AC_CHECK_LIB([lua], [lua_close], [have_liblua=yes], [have_liblua=no], [-lm])
   if test $have_liblua = yes; then
     AC_SUBST([liblua_CFLAGS], [])
     AC_SUBST([liblua_LIBS], [-llua])
   fi
   AC_LANG_POP(C)
fi
if test $have_liblua = yes; then
  AC_DEFINE([HAVE_LIBLUA], [1], [Define if liblua is installed])
fi
AM_CONDITIONAL([have_liblua], [test $have_liblua = yes])

# Check for PNG image library
#
PKG_CHECK_MODULES([libpng], [libpng],
  [have_libpng=yes],
  [have_libpng=no
   AC_MSG_RESULT([no])])
if test $have_libpng = yes; then
  AC_DEFINE([HAVE_LIBPNG], [1], [Define if libpng is installed])
fi
AM_CONDITIONAL([have_libpng], [test $have_libpng = yes])

# Check for OpenEXR image library
#
PKG_CHECK_MODULES([libexr], [OpenEXR],
  [have_libexr=yes],
  [have_libexr=no
   AC_MSG_RESULT([no])])
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
AM_PATH_LIB3DS([1.2.0], [have_lib3ds=yes])
if test $have_lib3ds = yes; then
  AC_DEFINE([HAVE_LIB3DS], [1], [Define if lib3ds is installed])
fi
AM_CONDITIONAL([have_lib3ds], [test $have_lib3ds = yes])

# See if lib3ds supports the "obj_flags" extension, which adds handling
# of LIB3DS_OBJ_ chunks (in particular the LIB3DS_OBJ_HIDDEN chunk,
# which is used by many real-world models).
#
# (currently this is my own extension to lib3ds; hopefully in the future
# it can get merged into the standard version)
#
if test $have_lib3ds = yes; then

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


##
## ----------------------------------------------------------------
## Some configuration options
##

lua_scene_def=no
if test -n "$SWIG" && test "$have_liblua" = yes; then
  lua_scene_def=yes
  AC_DEFINE([LUA_SCENE_DEF], [1], [Define if allowing lua scene definition files])
fi
AM_CONDITIONAL([lua_scene_def], [test $lua_scene_def = yes])


##
## ----------------------------------------------------------------
## Try to set optimization flags based on the type of CPU being used
##

if test "$GXX" = yes; then

  # These flags should always be accepted by g++
  #
  # The following settings are based on g++ 4.1 compiling for the pentium 4:
  #
  #   -O5 can result in some bloat, but does genuinely seem to be faster
  #   on our code than lower optimization levels.
  # 
  #   -fomit-frame-pointer seems to increase code size appreciably when
  #   used with -Os, so we omit it in that case.
  #
  AC_SUBST([CXX_OPT_FLAGS], ["-O5 -fomit-frame-pointer"])
  AC_SUBST([CXX_OPT_SIZE_FLAGS], ["-Os"])

  # g++ flags for architecture-independent floating-point optimization.
  #
  # When given the -ffast-math option, g++ will by default speculatively
  # evaluate floating-point expressions even if they may trap (e.g.,
  # divide).  The -ftrapping-math option causes g++ to be more careful
  # in such cases, so use that if we're going to enable floating-point
  # exceptions.
  #
  if test "$use_fp_exceptions" = yes; then
    CXX_MATH_OPT_FLAGS="-ffast-math"
    CXX_MATH_REQ_FLAGS="-ftrapping-math"
  else
    CXX_MATH_OPT_FLAGS="-ffast-math"
    CXX_MATH_REQ_FLAGS=""
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

  # See if -march=native worked.
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
	      sub (/^.*: */, ""); gsub (/ *\([^)]*\)/, ""); name = $0
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
	    15/65/*)
	      cpuinfo_cpu=opteron ;;
	    15/*)
	      cpuinfo_cpu=pentium4 ;;
	    6/9/*)
	      cpuinfo_cpu=pentium4 ;;
	  esac
	  ;;
      esac]

      AC_MSG_RESULT([$cpuinfo_cpu$cpuinfo_note])

      test "$cpuinfo_cpu" = unknown || host_cpu="$cpuinfo_cpu"
    fi

    case "$host_cpu" in
      i686|pentium3)
	CXX_MACH_FLAGS="-march=pentium3 -mfpmath=sse" ;;
      i786|pentium4)
	CXX_MACH_FLAGS="-march=pentium4 -mfpmath=sse" ;;
      pentium-m)
	CXX_MACH_FLAGS="-march=pentium-m -mfpmath=sse" ;;
      x86[-_]64)
	CXX_MACH_FLAGS="-march=k8 -mfpmath=sse" ;;
      opteron)
	CXX_MACH_FLAGS="-march=opteron -mfpmath=sse" ;;
      *)
	CXX_MACH_FLAGS="";;
    esac

    # Check machine options for validity; note that we check them all
    # together, not individually, as some may depend on others.
    #
    if test x"$CXX_MACH_FLAGS" != x; then
      SNOGRAY_CHECK_CXX_FLAG([CXX_MACH_FLAGS], [$CXX_MACH_FLAGS])
    fi
  fi

else

  AC_MSG_WARN([Not using GNU compiler, so not setting optimization flags])

  # This is accepted by almost every compiler.
  #
  AC_SUBST([CXX_OPT_FLAGS], [-O])

fi


##
## ----------------------------------------------------------------
## Final output
##


AC_CONFIG_FILES([Makefile])

AC_OUTPUT

m4_if(dnl	Do not change this comment
   arch-tag: 336310fb-78ae-4d49-94a3-d6ede54ce9dd
)dnl
