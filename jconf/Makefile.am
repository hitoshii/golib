## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(jlib_INCLUDE) \
	$(jio_INCLUDE)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libjconf.la

libjconf_la_SOURCES =  \
	jconf.c \
	node.c \
	type.c \
	parser.c


jconfsubincludedir=$(jlib_includedir)/jconf
jconfsubinclude_HEADERS= jconf.h \
	node.h \
	type.h \
	parser.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jconf-0.1.pc

EXTRA_DIST = \
	jconf-0.1.pc.in
