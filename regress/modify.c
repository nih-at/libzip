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

#ifndef HAVE_GETOPT
#include "getopt.h"
#endif

#include "zip.h"

const char *prg;

const char * const usage = "usage: %s [-cent] archive command1 [args] [command2 [args] ...]\n\n"
    "Supported options are:\n"
    "\t-c\tcheck consistency\n"
    "\t-e\terror if archive already exists (only useful with -n)\n"
    "\t-n\tcreate archive if it doesn't exist (default)\n"
    "\t-t\tdisregard current archive contents, if any\n"
    "\nSupported commands and arguments are:\n"
    "\tadd name content\n"
    "\tadd_dir name\n"
    "\tadd_file name file_to_add offset len\n"
    "\tadd_from_zip name archivename index offset len\n"
    "\tdelete index\n"
    "\tget_archive_comment\n"
    "\tget_file_comment index\n"
    "\trename index name\n"
    "\tset_file_comment index comment\n"
    "\nThe index is zero-based.\n";

int
main(int argc, char *argv[])
{
    const char *archive;
    struct zip *za, *z_in;
    struct zip_source *zs;
    char buf[100], c;
    int arg, err, flags, idx;

    arg = flags = 0;
    prg = argv[arg++];

    if (argc < 2) {
	fprintf(stderr, usage, prg);
	return 1;
    }

    while ((c=getopt(argc, argv, "cent")) != -1) {
	switch (c) {
	case 'c':
	    flags |= ZIP_CHECKCONS;
	    break;
	case 'e':
	    flags |= ZIP_EXCL;
	    break;
	case 'n':
	    flags |= ZIP_CREATE;
	    break;
	case 't':
	    flags |= ZIP_TRUNCATE;
	    break;

	default:
	    fprintf(stderr, usage, argv[0]);
	    return 1;
	}
    }

    archive = argv[arg++];

    if (flags == 0)
	flags = ZIP_CREATE;

    if ((za=zip_open(archive, flags, &err)) == NULL) {
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
	} else if (strcmp(argv[arg], "add_from_zip") == 0 && arg+5 < argc) {
	    /* add from another zip file */
	    int idx;
	    idx = atoi(argv[arg+3]);
	    if ((z_in=zip_open(argv[arg+2], ZIP_CHECKCONS, &err)) == NULL) {
		zip_error_to_str(buf, sizeof(buf), err, errno);
		fprintf(stderr, "can't open source zip archive `%s': %s\n", argv[arg+2], buf);
		err = 1;
		break;
	    }
	    if ((zs=zip_source_zip(za, z_in, idx, 0, atoi(argv[arg+4]), atoi(argv[arg+5]))) == NULL) {
		fprintf(stderr, "error creating file source from `%s' index '%d': %s\n", argv[arg+2], idx, zip_strerror(za));
		zip_close(z_in);
		err = 1;
		break;
	    }
	    if (zip_add(za, argv[arg+1], zs) == -1) {
		fprintf(stderr, "can't add file `%s': %s\n", argv[arg+1], zip_strerror(za));
		zip_source_free(zs);
		zip_close(z_in);
		err = 1;
		break;
	    }
	    arg += 6;
	} else if (strcmp(argv[arg], "delete") == 0 && arg+1 < argc) {
	    /* delete */
	    idx = atoi(argv[arg+1]);
	    if (zip_delete(za, idx) < 0) {
		fprintf(stderr, "can't delete file at index `%d': %s\n", idx, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 2;
	} else if (strcmp(argv[arg], "get_archive_comment") == 0) {
	    const char *comment;
	    int len;
	    /* get archive comment */
	    if ((comment=zip_get_archive_comment(za, &len, 0)) == NULL)
		printf("No archive comment\n");
	    else
		printf("Archive comment: %.*s\n", len, comment);
	    arg += 1;
	} else if (strcmp(argv[arg], "get_file_comment") == 0 && arg+1 < argc) {
	    const char *comment;
	    int len;
	    /* get file comment */
	    idx = atoi(argv[arg+1]);
	    if ((comment=zip_get_file_comment(za, idx, &len, 0)) == NULL) {
		fprintf(stderr, "can't get comment for `%s': %s\n", zip_get_name(za, idx, 0), zip_strerror(za));
		err = 1;
		break;
	    } else if (len == 0)
		printf("No comment for `%s'\n", zip_get_name(za, idx, 0));
	    else
		printf("File comment for `%s': %.*s\n", zip_get_name(za, idx, 0), len, comment);
	    arg += 2;
	} else if (strcmp(argv[arg], "rename") == 0 && arg+2 < argc) {
	    /* rename */
	    idx = atoi(argv[arg+1]);
	    if (zip_rename(za, idx, argv[arg+2]) < 0) {
		fprintf(stderr, "can't rename file at index `%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "set_file_comment") == 0 && arg+2 < argc) {
	    /* set file comment */
	    idx = atoi(argv[arg+1]);
	    if (zip_set_file_comment(za, idx, argv[arg+2], strlen(argv[arg+2])) < 0) {
		fprintf(stderr, "can't set file comment at index `%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
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
