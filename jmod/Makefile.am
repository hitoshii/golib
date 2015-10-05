## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
    -DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
    -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
    -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
    -Wall\
    -O0 \
    -g

lib_LTLIBRARIES = libjmod.la


libjmod_la_SOURCES = \
    jhook.c

libjmod_la_CFLAGS = $(jlib_INCLUDE)

libjmod_la_LDFLAGS =

libjmod_la_LIBADD =



jmodsubincludedir=$(jlib_includedir)/jmod
jmodsubinclude_HEADERS= \
    jmod.h \
    jhook.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jmod-0.1.pc

EXTRA_DIST = \
    jmod-0.1.pc.in

clean-local:
	-astyle --style=google -n *.c *.h
	-rm *~
