bin_PROGRAMS=zipcmp zipmerge ziptorrent

zipcmp_SOURCES=zipcmp.c dir.c parray.c parray_push.c parray_pop.c

zipcmp_CPPFLAGS=-I${top_srcdir}/lib -I../lib
zipcmp_LDADD=${top_builddir}/lib/libzip.la
zipmerge_CPPFLAGS=-I${top_srcdir}/lib -I../lib
zipmerge_LDADD=${top_builddir}/lib/libzip.la
ziptorrent_CPPFLAGS=-I${top_srcdir}/lib -I../lib
ziptorrent_LDADD=${top_builddir}/lib/libzip.la

EXTRA_DIST=	CMakeLists.txt getopt.c getopt.h compat.h dir.h parray.h
