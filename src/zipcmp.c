/*
  $NiH: zipcmp.c,v 1.5 2003/10/03 23:54:32 dillo Exp $

  cmpzip.c -- compare zip files
  Copyright (C) 2003 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include <errno.h>
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

char *usage = "usage: %s [-hViqv] zip1 zip2\n";

char help_head[] = PACKAGE " by Dieter Baron and Thomas Klausner\n\n";

char help[] = "\n\
  -h       display this help message\n\
  -V       display version number\n\
  -i       compare names ignoring case distinctions\n\
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

#define OPTIONS "hViqv"



static int entry_cmp(const void *p1, const void *p2);
static void entry_print(const void *p);
static int compare_list(const char *name[], int verbose,
		 const void *l[], const int n[], int size,
		 int (*cmp)(const void *, const void *),
		 void print(const void *));
static int compare_zip(const char *zn[], int verbose);

int ignore_case;



int
main(int argc, char *argv[])
{
    int verbose;
    int c;

    prg = argv[0];

    ignore_case = 0;
    verbose = 1;

    while ((c=getopt(argc, argv, OPTIONS)) != -1) {
	switch (c) {
	case 'i':
	    ignore_case = 1;
	    break;
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
    struct zip_stat st;
    struct entry *e[2];
    int n[2];
    int i, j;
    int err;
    char errstr[1024];

    for (i=0; i<2; i++) {
	if ((z=zip_open(zn[i], ZIP_CHECKCONS, &err)) == NULL) {
	    zip_error_str(errstr, sizeof(errstr), err, errno);
	    fprintf(stderr, "%s: cannot open zip file `%s': %s\n",
		    prg, zn[i], errstr);
	    return -1;
	}

	n[i] = zip_get_num_files(z);

	if ((e[i]=malloc(sizeof(*e[i]) * n[i])) == NULL) {
	    fprintf(stderr, "%s: malloc failure\n", prg);
	    exit(1);
	}

	for (j=0; j<n[i]; j++) {
	    zip_stat_index(z, i, &st);
	    e[i][j].name = strdup(st.name);
	    e[i][j].size = st.size;
	    e[i][j].crc = st.crc;
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

    if ((c=(ignore_case ? strcasecmp : strcmp)(e1->name, e2->name)) != 0)
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
