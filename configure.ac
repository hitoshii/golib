dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(libjlib, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC

LT_INIT

AC_PREFIX_DEFAULT(/usr)


# Check for pthread
PTHREAD_PRESENT=""
AC_CHECK_LIB( pthread, pthread_create, PTHREAD_PRESENT="yes",, $PTHREAD_LIBS
-lpthread )
if test "x$PTHREAD_PRESENT" = "xyes"; then
AC_DEFINE(HAVE_LIBPTHREAD, 1, [Define if libpthread is present])
PTHREAD_LIBS="-lpthread"
AC_SUBST(PTHREAD_LIBS)
fi

jlib_includedir='$(includedir)/jlib-0.1'
AC_SUBST(jlib_includedir)


jlib_INCLUDE='-I$(top_builddir)'
AC_SUBST(jlib_INCLUDE)
jlib_ADD='-L$(top_builddir)/jlib -ljlib'
AC_SUBST(jlib_ADD)

jconf_INCLUDE='-I$(top_builddir)'
AC_SUBST(jconf_INCLUDE)
jconf_ADD='-L$(top_builddir)/jconf -ljconf'
AC_SUBST(jconf_ADD)

jio_INCLUDE='-I$(top_builddir)'
AC_SUBST(jio_INCLUDE)
jio_ADD='-L$(top_builddir)/jio -ljio'
AC_SUBST(jio_ADD)

jmod_INCLUDE='-I$(top_builddir)'
AC_SUBST(jmod_INCLUDE)
jmod_ADD='-L$(top_builddir)/jmod -ljmod'
AC_SUBST(jmod_ADD)


AC_OUTPUT([
           Makefile
           jlib/jlib-0.1.pc
           jlib/Makefile
           jconf/Makefile
           jconf/jconf-0.1.pc
           jio/Makefile
           jio/jio-0.1.pc
           jmod/Makefile
           jmod/jmod-0.1.pc
           test/Makefile
           ])
