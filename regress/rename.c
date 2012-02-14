/*
  rename.c -- test cases for renaming files in zip archives
  Copyright (C) 2012 Dieter Baron and Thomas Klausner

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

const char *prg;



int
main(int argc, char *argv[])
{
    zip_uint64_t idx;
    int ze;
    struct zip *z;
    const char *archive;
    const char *oldname;
    const char *newname;

    prg = argv[0];

    if (argc != 4) {
        fprintf(stderr, "usage: %s archive oldname newname\n", prg);
        return 1;
    }

    archive = argv[1];
    oldname = argv[2];
    newname = argv[3];

    if ((z=zip_open(archive, 0, &ze)) == NULL) {
	char buf[100];
	zip_error_to_str(buf, sizeof(buf), ze, errno);
	printf("%s: can't open zip archive `%s': %s\n", prg,
	       archive, buf);
	return 1;
    }

    if ((idx=zip_name_locate(z, oldname, 0)) < 0) {
	printf("%s: unexpected error while looking for `%s': %s\n",
	       prg, oldname, zip_strerror(z));
	return 1;
    }

    if (zip_rename(z, idx, newname) < 0) {
	/* expected error, so no prg prefix */
	printf("error renaming `%s' to `%s': %s\n",
	       oldname, newname, zip_strerror(z));
	return 1;
    }

    if (zip_close(z) == -1) {
	fprintf(stderr, "%s: can't close zip archive `%s': %s\n", prg,
		archive, zip_strerror(z));
	return 1;
    }

    exit(0);
}
