bin_PROGRAMS=zipcmp zipmerge ziptool
noinst_PROGRAMS=hole

hole_SOURCES= hole.c source_hole.c
ziptool_SOURCES= source_hole.c ziptool.c

AM_CPPFLAGS=-I${top_srcdir}/lib -I../lib
LDADD=${top_builddir}/lib/libzip.la

EXTRA_DIST=	CMakeLists.txt getopt.c getopt.h compat.h
