## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
	-DPACKAGE_TEST_DIR=\""$(top_srcdir)/tests"\" \
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
	test-mod \
	test-log \
	test-socket \
	test-epoll \
	test-hashtable \
	test-loop \
	test-byte \
	test-conf \
	test-thread

TESTS=\
    test-string \
    test-file \
    test-mod \
    test-log \
    test-socket \
    test-epoll \
    test-hashtable \
    test-loop \
    test-byte \
    test-conf \
    test-thread

dist_check_DATA = \
    test.conf

test_conf_SOURCES = \
    test-conf.c

test_string_SOURCES = \
	test-string.c

test_file_SOURCES = \
	test-file.c

test_mod_SOURCES = \
	test-mod.c

test_log_SOURCES = \
	test-log.c

test_socket_SOURCES = \
	test-socket.c

test_epoll_SOURCES = \
	test-epoll.c

test_hashtable_SOURCES = \
    test-hashtable.c

test_loop_SOURCES = \
    test-loop.c

test_byte_SOURCES = \
	test-byte.c

test_thread_SOURCES = \
    test-thread.c

module: module.c
	gcc -shared -fPIC module.c -o module.so

all-local: module


clean-local:
	indent -kr -ts4 -nut *.c
	rm -rf hello-dir
	rm -f *~ *.so log.test
