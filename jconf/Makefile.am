## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(jlib_INCLUDE)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libjconf.la

libjconf_la_SOURCES =  \
	jconf.c


jconfsubincludedir=$(includedir)/jconf
jconfsubinclude_HEADERS= jconf.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jconf-0.1.pc

EXTRA_DIST = \
	jconf-0.1.pc.in
