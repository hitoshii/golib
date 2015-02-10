## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libjlib.la


libjlib_la_SOURCES = \
	jlist.c \
	jarray.c \
	jmem.c \
	jstring.c 

libjlib_la_LDFLAGS = 

libjlib_la_LIBADD = 



jlibsubincludedir=$(includedir)/jlib
jlibsubinclude_HEADERS= \
	jlib.h \
	jlist.h \
	jarray.h \
	jmem.h \
	jtypes.h \
	jstring.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jlib-0.1.pc

EXTRA_DIST = \
	jlib-0.1.pc.in
