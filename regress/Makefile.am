noinst_PROGRAMS=buffadd open name_locate fread
EXTRA_PROGRAMS=ziptest deltest

EXTRA_DIST=broken.zip test.zip

deltest_SOURCES=deltest.c
buffadd_SOURCES=buffadd.c
ziptest_SOURCES=ziptest.c

TESTS=buffadd open name_locate fread

AM_CPPFLAGS=-I${top_srcdir}/lib
LDADD=../lib/libzip.la
