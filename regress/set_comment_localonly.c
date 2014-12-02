/*
  set_comment_localonly.c -- set file comments
  Copyright (C) 2006-2014 Dieter Baron and Thomas Klausner

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
#include <string.h>

#include "zip.h"
#include "compat.h"

const char *prg;

int
main(int argc, char *argv[])
{
    const char *archive;
    zip_t *za;
    char buf[100];
    int err;
    zip_int64_t i;

    prg = argv[0];

    if (argc != 2) {
	fprintf(stderr, "usage: %s archive\n", prg);
	return 1;
    }

    archive = argv[1];
    
    if ((za=zip_open(archive, 0, &err)) == NULL) {
	zip_error_t error;
	zip_error_init_with_code(&error, err);
	fprintf(stderr, "%s: can't open zip archive '%s': %s\n", prg, archive, zip_error_strerror(&error));
	zip_error_fini(&error);
	return 1;
    }

    for (i=0; i<zip_get_num_entries(za, 0); i++) {
	snprintf(buf, sizeof(buf), "File comment no %" PRId64, i);
	if (zip_file_set_comment(za, (zip_uint64_t)i, buf, (zip_uint16_t)strlen(buf), 0) < 0) {
	    fprintf(stderr, "%s: zip_file_set_comment on file %" PRId64 " failed: %s\n", prg, i, zip_strerror(za));
	}
    }
    /* remove comment for third file */
    if (zip_file_set_comment(za, 2, NULL, 0, 0) < 0) {
	fprintf(stderr, "%s: zip_file_set_comment on file 2 failed: %s\n", prg, zip_strerror(za));
    }

    if (zip_close(za) == -1) {
	fprintf(stderr, "%s: can't close zip archive '%s': %s\n", prg, archive, zip_strerror(za));
	return 1;
    }

    return 0;
}
