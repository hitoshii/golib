## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
    -DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
    -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
    -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
    -Wall\
    -O0 \
    -g

lib_LTLIBRARIES = libjconf.la


libjconf_la_SOURCES = \
    jconfnode.c \
    jconfroot.c \
    jconfloader.c

libjconf_la_CFLAGS = $(jlib_INCLUDE)

libjconf_la_LDFLAGS =

libjconf_la_LIBADD =



jconfsubincludedir=$(jlib_includedir)/jconf
jconfsubinclude_HEADERS= \
    jconf.h \
    jconfnode.h \
    jconfroot.h \
    jconfloader.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jconf-0.1.pc

EXTRA_DIST = \
    jconf-0.1.pc.in

clean-local:
	-astyle --style=google -n *.c *.h
	-rm *~
