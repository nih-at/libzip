noinst_PROGRAMS=ziptest buffadd deltest

deltest_SOURCES=deltest.c
buffadd_SOURCES=buffadd.c
ziptest_SOURCES=ziptest.c error.c
noinst_HEADERS=error.h

TESTS=buffadd

INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/libzip
LDADD=../libzip.a
