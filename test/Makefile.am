## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
    -DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
    -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
    -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
    -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
    -DPACKAGE_TEST_DIR=\""$(top_srcdir)/test"\" \
    $(jlib_INCLUDE)

AM_CFLAGS =\
    -Wall\
    -g

AM_LDFLAGS =\
    $(jlib_ADD)

noinst_PROGRAMS= \
    test-string \
    test-hashtable \
    test-array \
    test-thread \
    test-main \
    test-message \
    test-quark \
    test-queue

TESTS=\
    test-string \
    test-hashtable \
    test-array \
    test-thread \
    test-main \
    test-message \
    test-quark \
    test-queue

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

test_quark_SOURCES = \
    test-quark.c

test_queue_SOURCES = \
    test-queue.c

clean-local:
	indent -kr -ts4 -nut *.c
	rm -f *~
