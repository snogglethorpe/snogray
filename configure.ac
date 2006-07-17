AC_INIT([snogray], [0.1], [Miles Bader <miles@gnu.org>])

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_PROG_RANLIB

AC_CHECK_HEADERS([fenv.h])

have_feenableexcept=no
AC_CHECK_LIB([m], [feenableexcept], [have_feenableexcept=yes])
if test $have_feenableexcept = yes; then
  AC_DEFINE([HAVE_FEENABLEEXCEPT], [1],
            [Define if feenableexcept is defined in -lm])
fi

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


AC_CONFIG_FILES([Makefile])

AC_OUTPUT

m4_if(dnl	Do not change this comment
   arch-tag: 336310fb-78ae-4d49-94a3-d6ede54ce9dd
)dnl
