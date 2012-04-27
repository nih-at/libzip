/*
  modify.c -- test case for modifying zip archive in multiple ways
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
#include <stdlib.h>
#include <string.h>

#include "zip.h"

const char *prg;

int
main(int argc, char *argv[])
{
    const char *archive;
    struct zip *za;
    struct zip_source *zs;
    char buf[100];
    int err;
    int arg;
    int idx;

    arg = 0;
    prg = argv[arg++];

    if (argc < 2) {
	fprintf(stderr, "usage: %s archive [add name content|add_dir name|add_file name filename offset len|delete index|file_comment index comment|rename index name]\n", prg);
	return 1;
    }

    archive = argv[arg++];
    
    if ((za=zip_open(archive, ZIP_CREATE, &err)) == NULL) {
	zip_error_to_str(buf, sizeof(buf), err, errno);
	fprintf(stderr, "can't open zip archive `%s': %s\n", archive, buf);
	return 1;
    }

    err = 0;
    while (arg < argc) {
	if (strcmp(argv[arg], "add") == 0 && arg+2 < argc) {
	    /* add */
    	    if ((zs=zip_source_buffer(za, argv[arg+2], strlen(argv[arg+2]), 0)) == NULL) {
		fprintf(stderr, "can't create zip_source from buffer: %s\n", zip_strerror(za));
		err = 1;
		break;
	    }

	    if (zip_add(za, argv[arg+1], zs) == -1) {
		zip_source_free(zs);
		fprintf(stderr, "can't add file `%s': %s\n", argv[arg+1], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "add_dir") == 0 && arg+1 < argc) {
	    /* add directory */
	    if (zip_add_dir(za, argv[arg+1]) < 0) {
		fprintf(stderr, "can't add directory `%s': %s\n", argv[arg+1], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 2;
	} else if (strcmp(argv[arg], "add_file") == 0 && arg+4 < argc) {
	    /* add */
    	    if ((zs=zip_source_file(za, argv[arg+2], atoi(argv[arg+3]), atoi(argv[arg+4]))) == NULL) {
		fprintf(stderr, "can't create zip_source from file: %s\n", zip_strerror(za));
		err = 1;
		break;
	    }

	    if (zip_add(za, argv[arg+1], zs) == -1) {
		zip_source_free(zs);
		fprintf(stderr, "can't add file `%s': %s\n", argv[arg+1], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 5;
	} else if (strcmp(argv[arg], "delete") == 0 && arg+1 < argc) {
	    /* delete */
	    idx = atoi(argv[arg+1]);
	    if (zip_delete(za, idx) < 0) {
		fprintf(stderr, "can't delete file at index `%d': %s\n", idx, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 2;
	} else if (strcmp(argv[arg], "file_comment") == 0 && arg+2 < argc) {
	    /* set file comment */
	    idx = atoi(argv[arg+1]);
	    if (zip_set_file_comment(za, idx, argv[arg+2], strlen(argv[arg+2])) < 0) {
		fprintf(stderr, "can't set file comment at index `%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "rename") == 0 && arg+2 < argc) {
	    /* rename */
	    idx = atoi(argv[arg+1]);
	    if (zip_rename(za, idx, argv[arg+2]) < 0) {
		fprintf(stderr, "can't rename file at index `%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else {
	    fprintf(stderr, "unrecognized command `%s', or not enough arguments\n", argv[arg]);
	    err = 1;
	    break;
	}
    }

    if (zip_close(za) == -1) {
	fprintf(stderr, "can't close zip archive `%s': %s\n", archive, zip_strerror(za));
	return 1;
    }

    return err;
}
