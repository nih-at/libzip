/*
  name_locate.c -- test cases for finding files in zip archives
  Copyright (C) 2005-2006 Dieter Baron and Thomas Klausner

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



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "zip.h"

#include "compat.h"

static int find_fail(struct zip *, const char *, int, int);
static int find_success(struct zip *, const char *, int);

const char *prg;



int
main(int argc, char *argv[])
{
    int fail, ze;
    struct zip *z;
    const char *archive;
    struct zip_source *s;
    const char buf[] = "teststring";

    fail = 0;
    prg = argv[0];

    if (argc != 2) {
        fprintf(stderr, "usage: %s archive\n", prg);
        return 1;
    }

    archive = argv[1];

    if ((z=zip_open(archive, 0, &ze)) == NULL) {
        char buf2[100];
        zip_error_to_str(buf2, sizeof(buf2), ze, errno);
	printf("%s: can't open zip archive '%s': %s\n", prg,
	       archive, buf2);
	return 1;
    }

    fail += find_fail(z, "nosuchfile", 0, ZIP_ER_NOENT);
    fail += find_success(z, "test", 0);
    fail += find_fail(z, "", 0, ZIP_ER_NOENT);
    fail += find_fail(z, "TeSt", 0, ZIP_ER_NOENT);
    fail += find_success(z, "TeSt", ZIP_FL_NOCASE);
    fail += find_success(z, "testdir/test2", 0);
    fail += find_success(z, "tesTdir/tESt2", ZIP_FL_NOCASE);
    fail += find_fail(z, "testdir/test2", ZIP_FL_NODIR, ZIP_ER_NOENT);
    fail += find_fail(z, "tesTdir/tESt2", ZIP_FL_NOCASE|ZIP_FL_NODIR,
		      ZIP_ER_NOENT);
    fail += find_fail(z, "test2", 0, ZIP_ER_NOENT);
    fail += find_success(z, "test2", ZIP_FL_NODIR);
    fail += find_success(z, "TeST2", ZIP_FL_NODIR|ZIP_FL_NOCASE);
    zip_delete(z, 0);
    fail += find_fail(z, "test", 0, ZIP_ER_NOENT);
    fail += find_success(z, "test", ZIP_FL_UNCHANGED);
    if ((s=zip_source_buffer(z, buf, sizeof(buf), 0)) == NULL || zip_file_add(z, "new", s, 0) < 0) {
	zip_source_free(s);
	printf("error adding file: %s\n", zip_strerror(z));
    }
    fail += find_success(z, "new", 0);
    fail += find_fail(z, "new", ZIP_FL_UNCHANGED, ZIP_ER_NOENT);
    if ((s=zip_source_buffer(z, buf, sizeof(buf), 0)) == NULL || zip_file_add(z, "", s, 0) < 0) {
	zip_source_free(s);
	printf("error adding file: %s\n", zip_strerror(z));
    }
    fail += find_success(z, "", 0);
    zip_unchange_all(z);
    fail += find_success(z, "test", 0);
    fail += find_fail(z, "new", 0, ZIP_ER_NOENT);

    if (zip_close(z) == -1) {
	fprintf(stderr, "%s: can't close zip archive '%s': %s\n", prg,
		archive, zip_strerror(z));
	return 1;
    }

    exit(fail ? 1 : 0);
}



static int
find_fail(struct zip *z, const char *name, int flags, int zerr)
{
    int ze, se;
    char expected[80];
    zip_int64_t idx;

    if ((idx=zip_name_locate(z, name, flags)) < 0) {
	zip_error_get(z, &ze, &se);
	if (ze != zerr) {
	    zip_error_to_str(expected, sizeof(expected), zerr, 0);
	    printf("unexpected error while looking for '%s' with flags %x: got `%s', expected `%s'\n",
		   name, flags, zip_strerror(z), expected);
	    return 1;
	}

	return 0;
    }

    printf("unexpected success while looking for '%s' with flags %x: index %" PRId64 "\n", name, flags, idx);
    return 1;
}



static int
find_success(struct zip *z, const char *name, int flags)
{

    if (zip_name_locate(z, name, flags) < 0) {
	printf("unexpected error while looking for '%s' with flags %x: %s\n",
	       name, flags, zip_strerror(z));
	return 1;
    }

    return 0;
}
