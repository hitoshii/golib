
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = jlib test

dist_doc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

uninstall-local:
	-rm -r $(docdir)
	-rm -r $(jlib_includedir)

install-exec-local:
	-ldconfig
