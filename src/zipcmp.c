/*
  $NiH: cmpzip.c,v 1.2 2003/03/16 10:21:37 wiz Exp $

  cmpzip.c -- compare zip files
  Copyright (C) 2003 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP files.
  The authors can be contacted at <nih@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "zip.h"

struct entry {
    char *name;
    unsigned int size;
    unsigned int crc;
};



char *prg;

char *usage = "usage: %s [-hVvq] zip1 zip2\n";

char help_head[] = PACKAGE " by Dieter Baron and Thomas Klausner\n\n";

char help[] = "\n\
  -h       display this help message\n\
  -V       display version number\n\
  -q       be quiet\n\
  -v       be verbose (print differences, default)\n\
\n\
Report bugs to <nih@giga.or.at>.\n";

char version_string[] = PACKAGE " " VERSION "\n\
Copyright (C) 2003 Dieter Baron and Thomas Klausner\n\
" PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n\
" PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

#define OPTIONS "hVqv"



static int entry_cmp(const void *p1, const void *p2);
static void entry_print(const void *p);
static int compare_list(const char *name[], int verbose,
		 const void *l[], const int n[], int size,
		 int (*cmp)(const void *, const void *),
		 void print(const void *));
static int compare_zip(const char *zn[], int verbose);



int
main(int argc, char *argv[])
{
    int verbose;
    int c;

    prg = argv[0];

    verbose = 1;

    while ((c=getopt(argc, argv, OPTIONS)) != -1) {
	switch (c) {
	case 'q':
	    verbose = 0;
	    break;
	case 'v':
	    verbose = 1;
	    break;

	case 'h':
	    fputs(help_head, stdout);
	    printf(usage, prg);
	    fputs(help, stdout);
	    exit(0);
	case 'V':
	    fputs(version_string, stdout);
	    exit(0);

	default:
	    fprintf(stderr, usage, prg);
	    exit(2);
	}
    }

    if (argc != optind+2) {
	fprintf(stderr, usage, prg);
	exit(2);
    }

    exit((compare_zip(argv+optind, verbose) == 0) ? 0 : 1);
}



static int
compare_zip(const char *zn[], int verbose)
{
    struct zip *z;
    struct entry *e[2];
    int n[2];
    int i, j;

    for (i=0; i<2; i++) {
	if ((z=zip_open(zn[i], ZIP_CHECKCONS)) == NULL) {
	    fprintf(stderr, "%s: cannot open zip file `%s': %s\n",
		    prg, zn[i], zip_err_str[zip_err]);
	    return -1;
	}

	n[i] = z->nentry;

	if ((e[i]=malloc(sizeof(*e[i]) * n[i])) == NULL) {
	    fprintf(stderr, "%s: malloc failure\n", prg);
	    exit(1);
	}

	for (j=0; j<n[i]; j++) {
	    e[i][j].name = strdup(z->entry[j].fn);
	    e[i][j].size = z->entry[j].meta->uncomp_size;
	    e[i][j].crc = z->entry[j].meta->crc;
	}

	zip_close(z);

	qsort(e[i], n[i], sizeof(e[i][0]), entry_cmp);
    }

    switch (compare_list(zn, verbose,
			 e, n, sizeof(e[i][0]),
			 entry_cmp, entry_print)) {
    case 0:
	exit(0);

    case 1:
	exit(1);

    default:
	exit(2);
    }
}



static int
compare_list(const char *name[2], int verbose,
	     const void *l[2], const int n[2], int size,
	     int (*cmp)(const void *, const void *),
	     void print(const void *))
{
    int i[2], j, c;
    int diff;

#define INC(k)	(i[k]++, l[k]+=size)
#define PRINT(k)	(((diff==0 && verbose)				    \
			     ? printf("--- %s\n+++ %s\n", name[0], name[1]) \
			     : 0),					    \
			 (verbose ? (printf("%c ", (k)?'+':'-'),	    \
				     print(l[k])) : 0),			    \
			 diff = 1)

    i[0] = i[1] = 0;
    diff = 0;
    while (i[0]<n[0] && i[1]<n[1]) {
	c = cmp(l[0], l[1]);

	if (c == 0) {
	    INC(0);
	    INC(1);
	}
	else if (c < 0) {
	    PRINT(0);
	    INC(0);
	}
	else {
	    PRINT(1);
	    INC(1);
	}
    }

    for (j=0; j<2; j++) {
	while (i[j]<n[j]) {
	    PRINT(j);
	    INC(j);
	}
    }

    return diff;
}



static int
entry_cmp(const void *p1, const void *p2)
{
    const struct entry *e1, *e2;
    int c;

    e1 = p1;
    e2 = p2;

    if ((c=strcmp(e1->name, e2->name)) != 0)
	return c;
    if (e1->size != e2->size)
	return e1->size - e2->size;
    if (e1->crc != e2->crc)
	return e1->crc - e2->crc;

    return 0;
}



static void
entry_print(const void *p)
{
    const struct entry *e;

    e = p;

    printf("%10u %08x %s\n", e->size, e->crc, e->name);
}
