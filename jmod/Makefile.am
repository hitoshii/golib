## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(jlib_INCLUDE) \
	$(jio_INCLUDE)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libjmod.la


libjmod_la_SOURCES = \
	jmod.c

libjmod_la_LDFLAGS =

libjmod_la_LIBADD =



jmodsubincludedir=$(jlib_includedir)/jmod

jmodsubinclude_HEADERS= \
	jmod.h \
	struct.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jmod-0.1.pc

EXTRA_DIST = \
	jmod-0.1.pc.in

clean-local:
	-indent -kr -ts4 -nut *.c *.h
	-rm *~
