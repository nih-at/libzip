noinst_PROGRAMS=buffadd open
EXTRA_PROGRAMS=ziptest deltest

EXTRA_DIST=test.zip

deltest_SOURCES=deltest.c
buffadd_SOURCES=buffadd.c
ziptest_SOURCES=ziptest.c

TESTS=buffadd open

INCLUDES=-I${top_srcdir}/lib
LDADD=../lib/libzip.la
