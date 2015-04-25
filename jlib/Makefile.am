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

lib_LTLIBRARIES = libjlib.la


libjlib_la_SOURCES = \
	jlist.c \
	jmem.c \
	jstrfuncs.c \
	jstring.c \
	jstack.c \
	jpath.c \
	jhashtable.c \
	jbytearray.c \
	jatomic.c \
	jthread.c

libjlib_la_LDFLAGS =

libjlib_la_LIBADD = $(PTHREAD_LIBS)



jlibsubincludedir=$(jlib_includedir)/jlib
jlibsubinclude_HEADERS= \
	jlib.h \
	jlist.h \
	jmem.h \
	jtypes.h \
	jstrfuncs.h \
	jstring.h \
	jstack.h \
	jpath.h \
	jhashtable.h \
	jbytearray.h \
	jtypes.h \
	jmacros.h \
	jatomic.h \
	jthread.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jlib-0.1.pc

EXTRA_DIST = \
	jlib-0.1.pc.in

clean-local:
	indent -kr -ts4 -nut *.c *.h
	rm *~
