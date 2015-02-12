## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = jlib jconf jio tests

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS



# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)

install-exec-local:
	ldconfig

runtest:
	@cd $(top_builddir)/tests && ./runtest
