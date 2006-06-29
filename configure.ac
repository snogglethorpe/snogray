AC_INIT([snogray], [0.1], [Miles Bader <miles@gnu.org>])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_PROG_RANLIB

PKG_PROG_PKG_CONFIG

# Check for PNG image library
#
have_libpng=no
if test x"$PKG_CONFIG" != x; then
  PKG_CHECK_MODULES([libpng], [libpng], [have_libpng=yes], [have_libpng=no])
fi
if test $have_libpng = yes; then
  AC_DEFINE([HAVE_LIBPNG], [1], [Define if libpng is installed])
fi
AM_CONDITIONAL([have_libpng], [test $have_libpng = yes])

# Check for OpenEXR image library
#
have_libexr=no
if test x"$PKG_CONFIG" != x; then
  PKG_CHECK_MODULES([libexr], [OpenEXR], [have_libexr=yes], [have_libexr=no])
fi
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

# Check for 3ds scene format library
#
have_lib3ds=no
AM_PATH_LIB3DS([1.2.0], [have_lib3ds=yes])
if test $have_lib3ds = yes; then
  AC_DEFINE([HAVE_LIB3DS], [1], [Define if lib3ds is installed])
fi
AM_CONDITIONAL([have_lib3ds], [test $have_lib3ds = yes])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

m4_if(dnl	Do not change this comment
   arch-tag: 336310fb-78ae-4d49-94a3-d6ede54ce9dd
)dnl
