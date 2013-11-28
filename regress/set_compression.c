/*
  set_compression.c -- set compression method for file in archive
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
#include <string.h>

#include "zipint.h"

#include "compat.h"

const char *prg;

int
main(int argc, char *argv[])
{
    const char *archive, *name;
    int method;
    struct zip *za;
    char buf[100];
    int err;
    zip_int64_t idx;

    prg = argv[0];

    if (argc != 4) {
	fprintf(stderr, "usage: %s archive file method\n", prg);
	return 1;
    }

    archive = argv[1];
    name = argv[2];
    method = atoi(argv[3]);
    
    if ((za=zip_open(archive, 0, &err)) == NULL) {
	zip_error_to_str(buf, sizeof(buf), err, errno);
	fprintf(stderr, "%s: can't open zip archive '%s': %s\n", prg,
		archive, buf);
	return 1;
    }
    if ((idx=zip_name_locate(za, name, 0)) < 0) {
	printf("%s: unexpected error while looking for '%s': %s\n",
	       prg, name, zip_strerror(za));
	return 1;
    }
    if (zip_set_file_compression(za, idx, method, /* TODO: add flags * when supported */ 0) < 0) {
	fprintf(stderr, "zip_set_file_compression with method '%d' on file %" PRId64 " failed: %s\n",
		method, idx, zip_strerror(za));
	return 1;
    }
    if (zip_close(za) == -1) {
	fprintf(stderr, "%s: can't close zip archive '%s': %s\n", prg, archive, zip_strerror(za));
	return 1;
    }

    return 0;
}
