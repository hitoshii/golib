dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(libjlib, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC


AC_PREFIX_DEFAULT(/usr)

jlib_INCLUDE='-I$(top_builddir)/jlib'
AC_SUBST(jlib_INCLUDE)
jlib_ADD='-L$(top_builddir)/jlib -ljlib'
AC_SUBST(jlib_ADD)


LT_INIT



AC_OUTPUT([
Makefile
jlib/jlib-0.1.pc
jlib/Makefile
jconf/Makefile
jconf/jconf-0.1.pc
tests/Makefile
])
