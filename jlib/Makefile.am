## Process this file with automake to produce Makefile.in

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
    jfileutils.c \
    jhashtable.c \
    jarray.c \
    jatomic.c \
    jthread.c \
    jepoll.c \
    jslist.c \
    jwakeup.c \
    jmain.c \
    jmessage.c \
    jprintf.c \
    jenviron.c \
    jthreadpool.c \
    jquark.c \
    jerror.c \
    jqueue.c \
    jasyncqueue.c \
    jtimer.c \
    jutils.c \
    junix.c \
    jobject.c

libjlib_la_LDFLAGS = -pthread

libjlib_la_LIBADD =



jlibsubincludedir=$(jlib_includedir)/jlib
jlibsubinclude_HEADERS= \
    jlib.h \
    jlist.h \
    jmem.h \
    jtypes.h \
    jstrfuncs.h \
    jstring.h \
    jstack.h \
    jfileutils.h \
    jhashtable.h \
    jarray.h \
    jmacros.h \
    jatomic.h \
    jthread.h \
    jepoll.h \
    jslist.h \
    jmain.h \
    jmessage.h \
    jprintf.h \
    jenviron.h \
    jthreadpool.h \
    jquark.h \
    jerror.h \
    jqueue.h \
    jasyncqueue.h \
    jtimer.h \
    ji18n.h \
    jutils.h \
    junix.h \
    jobject.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jlib-0.1.pc

EXTRA_DIST = \
    jlib-0.1.pc.in

clean-local:
	-indent -kr -ts4 -nut *.c *.h
	-rm *~
