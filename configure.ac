#This line defines the name of the output tarball
AC_INIT([jlib],[0.1])

# libtool
LT_INIT

#this line defines what compilation flags will be used
AM_INIT_AUTOMAKE

# DEFINE DEFAULT configure PREFIX
AC_PREFIX_DEFAULT(/usr)

#this line defines what libraries should be used during building
# DEFINE YOUR OWN LIBRARY CHECK - DO NOT DELETE THIS LINE
AC_CHECK_LIB([pthread],[pthread_create], ,[AC_MSG_ERROR(You need to have pthread)])


AC_PROG_CC

jlib_includedir='$(includedir)/jlib-0.1'
AC_SUBST(jlib_includedir)


jlib_INCLUDE='-I$(top_builddir)'
AC_SUBST(jlib_INCLUDE)
jlib_ADD='-L$(top_builddir)/jlib -ljlib'
AC_SUBST(jlib_ADD)


#This line defines where Makefiles are stored for projects and sources
AC_OUTPUT([
           Makefile
           jlib/jlib-0.1.pc
           jlib/Makefile
           test/Makefile
           ])
