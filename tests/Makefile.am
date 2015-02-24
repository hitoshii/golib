## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
	$(jlib_INCLUDE)

AM_CFLAGS =\
	 -Wall\
	 -g

AM_LDFLAGS =\
	$(jlib_ADD) \
	$(jconf_ADD) \
	$(jio_ADD) \
	$(jmod_ADD)

noinst_PROGRAMS= \
	test-string \
	test-file \
	test-conf \
	test-mod

noinst_SCRIPTS=runtest

test_string_SOURCES = \
	string.c

test_file_SOURCES = \
	file.c

test_conf_SOURCES = \
	conf.c

test_mod_SOURCES = \
	mod.c

module: module.c
	gcc -shared -fPIC module.c -o module.so

all-local: module


clean-local:
	indent -kr -ts4 -nut *.c
	rm -rf hello-dir
	rm -f *~ *.so
