bin_PROGRAMS=zipcmp zipmerge

zipcmp_SOURCES=zipcmp.c

zipcmp_CPPFLAGS=-I${top_srcdir}/lib -I../lib
zipcmp_LDADD=${top_builddir}/lib/libzip.la
zipmerge_CPPFLAGS=-I${top_srcdir}/lib -I../lib
zipmerge_LDADD=${top_builddir}/lib/libzip.la

EXTRA_DIST=	CMakeLists.txt getopt.c getopt.h compat.h
