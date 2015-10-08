## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
    -DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
    -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
    -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
    -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
    -DPACKAGE_TEST_DIR=\""$(top_srcdir)/test"\" \
    $(jlib_INCLUDE)

AM_CFLAGS =\
    -Wall\
    -g \
    -O0

AM_LDFLAGS =\
    $(jlib_ADD) \
    $(jio_ADD) \
    $(jconf_ADD) \
    $(jmod_ADD)

noinst_PROGRAMS= \
    test-string \
    test-hashtable \
    test-array \
    test-thread \
    test-main \
    test-message \
    test-queue \
    test-asyncqueue \
    test-threadpool \
    test-socketaddress \
    test-socket \
    test-listen \
    test-inputstream \
    test-outputstream \
    test-conf \
    test-module

TESTS=\
    test-string \
    test-hashtable \
    test-array \
    test-thread \
    test-main \
    test-message \
    test-queue \
    test-asyncqueue \
    test-threadpool \
    test-socketaddress \
    test-socket \
    test-listen \
    test-inputstream \
    test-outputstream \
    test-conf \
    test-module

test_string_SOURCES = \
	test-string.c

test_hashtable_SOURCES = \
    test-hashtable.c

test_array_SOURCES = \
	test-array.c

test_thread_SOURCES = \
    test-thread.c

test_main_SOURCES = \
    test-main.c

test_message_SOURCES = \
    test-message.c

test_queue_SOURCES = \
    test-queue.c

test_asyncqueue_SOURCES = \
    test-asyncqueue.c

test_threadpool_SOURCES = \
    test-threadpool.c

test_socketaddress_SOURCES = \
    test-socketaddress.c

test_socket_SOURCES = \
    test-socket.c

test_listen_SOURCES = \
    test-listen.c

test_inputstream_SOURCES = \
    test-inputstream.c

test_outputstream_SOURCES = \
    test-outputstream.c

test_conf_SOURCES = \
    test-conf.c

test_module_SOURCES = \
    test-module.c

dist_check_DATA = \
    test.conf \
    conf.d/test2.conf

.PHONY: test-module-mod

test-module-mod: test-module-mod.c
	$(CC) $(AM_CPPFLAGS) -shared -fPIC test-module-mod.c -o test-mod.so

all-local: test-module-mod

clean-local:
	-astyle --style=google -n *.c *.h
	-rm -rf *~ test-mod.so
