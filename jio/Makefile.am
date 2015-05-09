## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -O0 \
	 -g

lib_LTLIBRARIES = libjio.la


libjio_la_SOURCES = \
	jfile.c \
	jdir.c \
	jsocket.c \
	jpoll.c 

libjio_la_LDFLAGS =

libjio_la_LIBADD =



jiosubincludedir=$(jlib_includedir)/jio
jiosubinclude_HEADERS= \
	jfile.h \
	jdir.h \
	jio.h \
	jsocket.h \
	jpoll.h 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jio-0.1.pc

EXTRA_DIST = \
	jio-0.1.pc.in


clean-local:
	indent -kr -ts4 -nut *.c *.h
	rm *~
