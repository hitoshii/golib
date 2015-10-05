#This line defines the name of the output tarball
AC_INIT([jlib],[0.1])

# libtool
LT_INIT

#this line defines what compilation flags will be used
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

# DEFINE DEFAULT configure PREFIX
AC_PREFIX_DEFAULT(/usr)

#this line defines what libraries should be used during building
# DEFINE YOUR OWN LIBRARY CHECK - DO NOT DELETE THIS LINE
AC_CHECK_LIB([pthread],[pthread_create], ,[AC_MSG_ERROR(You need to have pthread)])


AC_CONFIG_MACRO_DIR(m4)

AC_PROG_CC

jlib_includedir='$(includedir)/jlib-0.1'
AC_SUBST(jlib_includedir)


jlib_INCLUDE='-I$(top_builddir)'
AC_SUBST(jlib_INCLUDE)
jlib_ADD='-L$(top_builddir)/jlib -ljlib'
AC_SUBST(jlib_ADD)

jio_ADD='-L$(top_builddir)/jio -ljio'
AC_SUBST(jio_ADD)

jconf_ADD='-L$(top_builddir)/jconf -ljconf'
AC_SUBST(jconf_ADD)


#This line defines where Makefiles are stored for projects and sources
AC_OUTPUT([
Makefile
jlib/jlib-0.1.pc
jlib/Makefile
jio/jio-0.1.pc
jio/Makefile
jconf/Makefile
jconf/jconf-0.1.pc
jmod/Makefile
jmod/jmod-0.1.pc
test/Makefile
])
