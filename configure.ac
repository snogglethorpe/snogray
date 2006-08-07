AC_INIT([snogray], [0.1], [Miles Bader <miles@gnu.org>])

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])


AC_PROG_CXX
AC_PROG_RANLIB


# We use C++ for everything, so do all our tests using that too.
#
AC_LANG(C++)


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
      AC_SUBST([$1], [$2])
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


AC_CHECK_HEADERS([fenv.h])

have_feenableexcept=no
AC_CHECK_LIB([m], [feenableexcept], [have_feenableexcept=yes])
if test $have_feenableexcept = yes; then
  AC_DEFINE([HAVE_FEENABLEEXCEPT], [1],
            [Define if feenableexcept is defined in -lm])
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

  _OLD_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $LIB3DS_CFLAGS"

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

  CFLAGS="$_OLD_CFLAGS"
fi


##
## ----------------------------------------------------------------
## Try to set optimization flags based on the type of CPU being used
##

if test "$GXX" = yes; then

  # We try to use the host type to set optimization flags
  #
  AC_CANONICAL_HOST
  
  # If this is a linux system with /proc/cpuinfo we can be a bit more specific
  #
  if test -r /proc/cpuinfo; then
    AC_MSG_CHECKING([CPU type in /proc/cpuinfo])

    cpuinfo_cpu=unknown
    cpuinfo_note=''
   
    case "$host_cpu" in
      i?86)
	cpuinfo=[`$AWK '
          /^cpu family[ \t]*:/ { family = $NF; next }
	  /^model[ \t]*:/ { model = $NF; next }
	  /^model name[ \t]*:/ {
	    sub (/^.*: */, ""); gsub (/ *\([^)]*\)/, ""); name = $0
	    next
	  }
          END { print family "/" model "/" name }
	' /proc/cpuinfo`]

	cpuinfo_name=`basename "$cpuinfo"`
	cpuinfo_note=" ($cpuinfo_name)"

	case "$cpuinfo" in
	  15/*)
	    cpuinfo_cpu=pentium4 ;;
	  "*Pentium III*")
	    cpuinfo_cpu=pentium3 ;;
	  "*Pentium 4*")
	    cpuinfo_cpu=pentium4 ;;
	esac
	;;
    esac

    AC_MSG_RESULT([$cpuinfo_cpu$cpuinfo_note])

    test "$cpuinfo_cpu" = unknown || host_cpu="$cpuinfo_cpu"
  fi

  # These flags should always be accepted by g++
  #
  AC_SUBST([CXX_OPT_FLAGS], ["-O5 -fomit-frame-pointer"])

  # Some optimization flags which not be supported.
  #
  SNOGRAY_CHECK_CXX_FLAGS([CXX_EXTRA_OPT_FLAGS], [-ffast-math])

  # Guess machine-specific options based on cpu type.
  #
  case "$host_cpu" in
    i686|pentium3)
      CXX_MACH_FLAGS="-march=pentium3 -mfpmath=sse -msse" ;;
    i786|pentium4)
      CXX_MACH_FLAGS="-march=pentium4 -mfpmath=sse -msse2" ;;
    *)
      CXX_MACH_FLAGS="";;
  esac

  # Check machine options for validity; note that we check them all
  # together, not individually, as some may depend on others.
  #
  SNOGRAY_CHECK_CXX_FLAG([CXX_MACH_FLAGS], [$CXX_MACH_FLAGS])

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
