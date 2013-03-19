/*
  zipcmp.c -- compare zip files
  Copyright (C) 2003-2013 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

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



#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <zlib.h>

#ifndef HAVE_GETOPT
#include "getopt.h"
#endif

/* include zipint.h for Windows compatibility */
#include "zipint.h"
#include "zip.h"
#include "compat.h"

struct ef {
    const char *name;
    zip_uint16_t flags;
    zip_uint16_t id;
    zip_uint16_t size;
    const zip_uint8_t *data;
};

struct entry {
    char *name;
    zip_uint64_t size;
    zip_uint32_t crc;
    zip_uint32_t comp_method;
    struct ef *extra_fields;
    int n_extra_fields;
    const char *comment;
    zip_uint32_t comment_length;
};



const char *prg;

#define PROGRAM	"zipcmp"

const char *usage = "usage: %s [-hipqtVv] zip1 zip2\n";

char help_head[] =
    PROGRAM " (" PACKAGE ") by Dieter Baron and Thomas Klausner\n\n";

char help[] = "\n\
  -h       display this help message\n\
  -V       display version number\n\
  -i       compare names ignoring case distinctions\n\
  -p       compare as many details as possible\n\
  -q       be quiet\n\
  -t       test zip files\n\
  -v       be verbose (print differences, default)\n\
\n\
Report bugs to <libzip@nih.at>.\n";

char version_string[] = PROGRAM " (" PACKAGE " " VERSION ")\n\
Copyright (C) 2013 Dieter Baron and Thomas Klausner\n\
" PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n";

#define OPTIONS "hVipqtv"



static int ef_read(struct zip *za, int idx, struct entry *e);
static int ef_compare(char *const name[2], const struct entry *e1, const struct entry *e2);
static int ef_order(const void *a, const void *b);
static void ef_print(const void *p);
static int entry_cmp(const void *p1, const void *p2);
static int entry_paranoia_checks(char *const name[2], const void *p1, const void *p2);
static void entry_print(const void *p);
static int comment_compare(const char *c1, int l1, const char *c2, int l2);
static int compare_list(char * const name[],
			const void *l[], const int n[], int size,
			int (*cmp)(const void *, const void *),
			int (*checks)(char *const name[2], const void *, const void *),
			void (*print)(const void *));
static int compare_zip(char * const zn[]);
static int test_file(struct zip *za, int idx, zip_int64_t size, unsigned int crc);

int ignore_case, test_files, paranoid, verbose;
int header_done;



int
main(int argc, char * const argv[])
{
    int c;

    prg = argv[0];

    ignore_case = 0;
    test_files = 0;
    paranoid = 0;
    verbose = 1;

    while ((c=getopt(argc, argv, OPTIONS)) != -1) {
	switch (c) {
	case 'i':
	    ignore_case = 1;
	    break;
	case 'p':
	    paranoid = 1;
	    break;
	case 'q':
	    verbose = 0;
	    break;
	case 't':
	    test_files = 1;
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

    exit((compare_zip(argv+optind) == 0) ? 0 : 1);
}



static int
compare_zip(char * const zn[])
{
    struct zip *za, *z[2];
    struct zip_stat st;
    struct entry *e[2];
    int n[2];
    int i, j;
    int err;
    char errstr[1024];
    int res;

    const char *archive_comment[2];
    int archive_comment_len[2];

    for (i=0; i<2; i++) {
	if ((za=zip_open(zn[i], paranoid ? ZIP_CHECKCONS : 0, &err)) == NULL) {
	    zip_error_to_str(errstr, sizeof(errstr), err, errno);
	    fprintf(stderr, "%s: cannot open zip archive `%s': %s\n",
		    prg, zn[i], errstr);
	    return -1;
	}

	z[i] = za;
	n[i] = zip_get_num_entries(za, 0);

	if (n[i] == 0)
	    e[i] = NULL;
        else {
	    if ((e[i]=(struct entry *)malloc(sizeof(*e[i]) * n[i])) == NULL) {
	        fprintf(stderr, "%s: malloc failure\n", prg);
	        exit(1);
	    }

	    for (j=0; j<n[i]; j++) {
	        zip_stat_index(za, j, 0, &st);
	        e[i][j].name = strdup(st.name);
	        e[i][j].size = st.size;
	        e[i][j].crc = st.crc;
	        if (test_files)
		    test_file(za, j, st.size, st.crc);
		if (paranoid) {
		    e[i][j].comp_method = st.comp_method;
		    ef_read(za, j, e[i]+j);
		    e[i][j].comment = zip_file_get_comment(za, j, &e[i][j].comment_length, 0);
		    
		}
		else {
		    e[i][j].comp_method = 0;
		    e[i][j].n_extra_fields = 0;
		}
	    }
	    qsort(e[i], n[i], sizeof(e[i][0]), entry_cmp);
        }

	if (paranoid)
	    archive_comment[i] = zip_get_archive_comment(za, &archive_comment_len[i], 0);
        else {
            archive_comment[i] = NULL;
            archive_comment_len[i] = 0;
        }
    }

    header_done = 0;

    res = compare_list(zn, (const void **)e, n, sizeof(e[i][0]),
		       entry_cmp, paranoid ? entry_paranoia_checks : NULL, entry_print);

    if (paranoid) {
	if (comment_compare(archive_comment[0], archive_comment_len[0], archive_comment[1], archive_comment_len[1]) != 0) {
	    if (verbose) {
		printf("--- archive comment (%d)\n", archive_comment_len[0]);
		printf("+++ archive comment (%d)\n", archive_comment_len[1]);
	    }
	    res = 1;
	}
    }

    for (i=0; i<2; i++)
	zip_close(z[i]);

    switch (res) {
    case 0:
	exit(0);

    case 1:
	exit(1);

    default:
	exit(2);
    }
}



static int
comment_compare(const char *c1, int l1, const char *c2, int l2) {
    if (l1 != l2)
	return 1;

    if (l1 == 0)
	return 0;

    if (c1 == NULL || c2 == NULL)
        return c1 == c2;
    
    return memcmp(c1, c2, l2);
}



static int
compare_list(char * const name[2],
	     const void *l[2], const int n[2], int size,
	     int (*cmp)(const void *, const void *),
	     int (*check)(char *const name[2], const void *, const void *),
	     void (*print)(const void *))
{
    int i[2], j, c;
    int diff;

#define INC(k)	(i[k]++, l[k]=((const char *)l[k])+size)
#define PRINT(k)	do {						\
			    if (header_done==0 && verbose) {		\
			        printf("--- %s\n+++ %s\n", name[0], name[1]); \
				header_done = 1;			\
			    }						\
			    if (verbose) {				\
			        printf("%c ", (k)?'+':'-');		\
				print(l[k]);				\
			    }						\
			    diff = 1;					\
			} while (0)

    i[0] = i[1] = 0;
    diff = 0;
    while (i[0]<n[0] && i[1]<n[1]) {
	c = cmp(l[0], l[1]);

	if (c == 0) {
	    if (check)
		diff |= check(name, l[0], l[1]);
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
ef_read(struct zip *za, int idx, struct entry *e)
{
    int n_local, n_central;
    int i;

    n_local = zip_file_extra_fields_count(za, idx, ZIP_FL_LOCAL);
    n_central = zip_file_extra_fields_count(za, idx, ZIP_FL_CENTRAL);
    e->n_extra_fields = n_local + n_central;
    
    if ((e->extra_fields=(struct ef *)malloc(sizeof(e->extra_fields[0])*e->n_extra_fields)) == NULL)
	return -1;

    for (i=0; i<n_local; i++) {
	e->extra_fields[i].name = e->name;
	if ((e->extra_fields[i].data=zip_file_extra_field_get(za, idx, i, &e->extra_fields[i].id, &e->extra_fields[i].size, ZIP_FL_LOCAL)) == NULL)
	    return -1;
	e->extra_fields[i].flags = ZIP_FL_LOCAL;
    }
    for (; i<e->n_extra_fields; i++) {
	e->extra_fields[i].name = e->name;
	if ((e->extra_fields[i].data=zip_file_extra_field_get(za, idx, i-n_local, &e->extra_fields[i].id, &e->extra_fields[i].size, ZIP_FL_CENTRAL)) == NULL)
	    return -1;
	e->extra_fields[i].flags = ZIP_FL_CENTRAL;
    }

    qsort(e->extra_fields, e->n_extra_fields, sizeof(e->extra_fields[0]), ef_order);

    return 0;
}



static int
ef_compare(char *const name[2], const struct entry *e1, const struct entry *e2)
{
    struct ef *ef[2];
    int n[2];

    ef[0] = e1->extra_fields;
    ef[1] = e2->extra_fields;
    n[0] = e1->n_extra_fields;
    n[1] = e2->n_extra_fields;

    return compare_list(name, (const void **)ef, n, sizeof(struct ef), ef_order, NULL, ef_print);
}




static int
ef_order(const void *ap, const void *bp) {
    const struct ef *a, *b;

    a = (struct ef *)ap;
    b = (struct ef *)bp;

    if (a->flags != b->flags)
	return a->flags - b->flags;
    if (a->id != b->id)
	return a->id - b->id;
    if (a->size != b->size)
	return a->size - b->size;
    return memcmp(a->data, b->data, a->size);
}



static void
ef_print(const void *p)
{
    const struct ef *ef = (struct ef *)p;
    int i;

    printf("                    %s  ", ef->name);
    printf("%04x %c <", ef->id, ef->flags == ZIP_FL_LOCAL ? 'l' : 'c');
    for (i=0; i<ef->size; i++)
	printf("%s%02x", i ? " " : "", ef->data[i]);
    printf(">\n");
}



static int
entry_cmp(const void *p1, const void *p2)
{
    const struct entry *e1, *e2;
    int c;

    e1 = (struct entry *)p1;
    e2 = (struct entry *)p2;

    if ((c=(ignore_case ? strcasecmp : strcmp)(e1->name, e2->name)) != 0)
	return c;
    if (e1->size != e2->size) {
        if (e1->size > e2->size)
            return 1;
        else
            return -1;
    }
    if (e1->crc != e2->crc)
	return e1->crc - e2->crc;

    return 0;
}



static int
entry_paranoia_checks(char *const name[2], const void *p1, const void *p2) {
    const struct entry *e1, *e2;
    int ret;

    e1 = (struct entry *)p1;
    e2 = (struct entry *)p2;

    ret = 0;

    if (ef_compare(name, e1, e2) != 0)
	ret = 1;

    if (e1->comp_method != e2->comp_method) {
	if (verbose) {
	    if (header_done==0) {
		printf("--- %s\n+++ %s\n", name[0], name[1]);
		header_done = 1;
	    }
	    printf("---                     %s  ", e1->name);
	    printf("method %u\n", e1->comp_method);
	    printf("+++                     %s  ", e1->name);
	    printf("method %u\n", e2->comp_method);
	}
	ret =  1;
    }
    if (comment_compare(e1->comment, e1->comment_length, e2->comment, e2->comment_length) != 0) {
	if (verbose) {
	    if (header_done==0) {
		printf("--- %s\n+++ %s\n", name[0], name[1]);
		header_done = 1;
	    }
	    printf("---                     %s  ", e1->name);
	    printf("comment %d\n", e1->comment_length);
	    printf("+++                     %s  ", e1->name);
	    printf("comment %d\n", e2->comment_length);
	}
	ret = 1;
    }

    return ret;
}




static void
entry_print(const void *p)
{
    const struct entry *e;

    e = (struct entry *)p;

    /* XXX PRId64 */
    printf("%10lu %08x %s\n", (unsigned long)e->size, e->crc, e->name);
}



static int
test_file(struct zip *za, int idx, zip_int64_t size, unsigned int crc)
{
    struct zip_file *zf;
    char buf[8192];
    zip_int64_t n, nsize;
    zip_uint32_t ncrc;
    
    if ((zf=zip_fopen_index(za, idx, 0)) == NULL) {
	fprintf(stderr, "%s: cannot open file %d in archive: %s\n",
		prg, idx, zip_strerror(za));
	return -1;
    }

    ncrc = (zip_uint32_t)crc32(0, NULL, 0);
    nsize = 0;
    
    while ((n=zip_fread(zf, buf, sizeof(buf))) > 0) {
	nsize += n;
	ncrc = (zip_uint32_t)crc32(ncrc, (const Bytef *)buf, (unsigned int)n);
    }

    if (n < 0) {
	fprintf(stderr, "%s: error reading file %d in archive: %s\n",
		prg, idx, zip_file_strerror(zf));
	zip_fclose(zf);
	return -1;
    }

    zip_fclose(zf);

    if (nsize != size) {
	fprintf(stderr, "%s: file %d: unexpected length %" PRId64 " (should be %" PRId64 ")\n",
		prg, idx, nsize, size);
	return -2;
    }
    if (ncrc != crc) {
	fprintf(stderr, "%s: file %d: unexpected length %x (should be %x)\n",
		prg, idx, ncrc, crc);
	return -2;
    }

    return 0;
}
