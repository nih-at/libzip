/*
  $NiH: buffadd.c,v 1.4.4.1 2004/04/10 23:52:40 dillo Exp $

  buffadd.c -- test cases for adding files from buffer
  Copyright (C) 1999, 2003 Dieter Baron and Thomas Klausner

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

#include "zip.h"

char *teststr="This is a test, and it seems to have been successful.\n";
char *testname="testfile.txt";
char *testzip="test_zip.zip";

int
main(int argc, char *argv[])
{
    struct zip *z;
    struct zip_file *ze;
    int err;
    
    char buf[2000];

    remove(testzip);
    
    if ((z=zip_open(testzip, ZIP_CREATE, &err)) == NULL) {
	zip_error_str(buf, sizeof(buf), err, errno);
	fprintf(stderr,"%s: can't open zipfile %s: %s\n", argv[0],
		testzip, buf);
	exit(1);
    }

    if (zip_add_data(z, testname, teststr, strlen(teststr), 0)==-1) {
	fprintf(stderr,"%s: can't add buffer '%s': %s\n", argv[0],
		teststr, zip_strerror(z));
	exit(1);
    }

    if (zip_close(z) == -1) {
	fprintf(stderr,"%s: can't close zipfile %s\n", argv[0],
		testzip);
	exit(1);
    }

    if ((z=zip_open(testzip, ZIP_CHECKCONS, &err))==NULL) {
	zip_error_str(buf, sizeof(buf), err, errno);
	fprintf(stderr,"%s: can't re-open zipfile %s: %s\n", argv[0],
		testzip, buf);
	exit(1);
    }

    if ((ze=zip_fopen(z, testname, 0))==NULL) {
	fprintf(stderr,"%s: can't fopen file '%s' in '%s': %s\n", argv[0],
		testname, testzip, zip_strerror(z));
	exit(1);
    }

    if (zip_fread(ze, buf, 2000) < 0) {
	fprintf(stderr,"%s: can't read from '%s' in zipfile '%s': %s\n",
		argv[0], testname, testzip, zip_file_strerror(ze));
	exit(1);
    }
    
    if (strcmp(buf, teststr)) {
	fprintf(stderr,"%s: wrong data: '%s' instead of '%s'\n", argv[0],
		buf, teststr);
	exit(1);
    }

    if (zip_close(z) == -1) {
	fprintf(stderr,"%s: can't close zipfile %s\n", argv[0],
		testzip);
	exit(1);
    }

    remove(testzip);
    
    return 0;
}
