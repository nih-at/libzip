noinst_PROGRAMS=ziptest buffadd deltest

deltest_SOURCES=deltest.c
buffadd_SOURCES=buffadd.c
ziptest_SOURCES=ziptest.c

TESTS=buffadd

AM_CPPFLAGS=-I$(top_srcdir) -I$(top_srcdir)/libzip
LDADD=../libzip.la
