noinst_PROGRAMS=ziptest

ziptest_SOURCES=ziptest.c error.c
noinst_HEADERS=error.h

INCLUDES=-I.. -I../..
LDADD=-L.. -lzip

ziptest: ../libzip.a
