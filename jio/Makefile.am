## Process this file with automake to produce Makefile.in

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
    jsocket.c \
    jsocketaddress.c \
    jpoll.c \
    jfile.c \
    jinputstream.c \
    joutputstream.c \
    jfileinputstream.c \
    jfileoutputstream.c \
    jbufferedinputstream.c 

libjio_la_CFLAGS = $(jlib_INCLUDE)

libjio_la_LDFLAGS =

libjio_la_LIBADD =



jiosubincludedir=$(jlib_includedir)/jio
jiosubinclude_HEADERS= \
    jio.h \
    jsocket.h \
    jioenum.h \
    jsocketaddress.h \
    jpoll.h \
    jfile.h \
    jinputstream.h \
    joutputstream.h \
    jfileinputstream.h \
    jfileoutputstream.h \
    jbufferedinputstream.h 


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jio-0.1.pc

EXTRA_DIST = \
    jio-0.1.pc.in

clean-local:
	-astyle --style=google -n *.c *.h
	-rm *~
