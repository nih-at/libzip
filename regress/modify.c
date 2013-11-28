/*
  modify.c -- test tool for modifying zip archive in multiple ways
  Copyright (C) 2012-2013 Dieter Baron and Thomas Klausner

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
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

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
    "\tcount_extra index flags\n"
    "\tcount_extra_by_id index extra_id flags\n"
    "\tdelete index\n"
    "\tdelete_extra index extra_idx flags\n"
    "\tdelete_extra_by_id index extra_id extra_index flags\n"
    "\tget_archive_comment\n"
    "\tget_extra index extra_index flags\n"
    "\tget_extra_by_id index extra_id extra_index flags\n"
    "\tget_file_comment index\n"
    "\trename index name\n"
    "\tset_extra index extra_id extra_index flags value\n"
    "\tset_file_comment index comment\n"
    "\tset_file_compression index method flags\n"
    "\nThe index is zero-based.\n";

static zip_flags_t
get_flags(const char *arg)
{
    zip_flags_t flags = 0;
    if (strchr(arg, 'c') != NULL)
	flags |= ZIP_FL_CENTRAL;
    if (strchr(arg, 'l') != NULL)
	flags |= ZIP_FL_LOCAL;
    if (strchr(arg, 'u') != NULL)
	flags |= ZIP_FL_UNCHANGED;
    return flags;
}

static zip_int32_t
get_compression_method(const char *arg)
{
    if (strcmp(arg, "default") == 0)
        return ZIP_CM_DEFAULT;
    else if (strcmp(arg, "store") == 0)
        return ZIP_CM_STORE;
    else if (strcmp(arg, "deflate") ==0)
        return ZIP_CM_DEFLATE;
    return 0; /* TODO: error handling */
}

static void
hexdump(const zip_uint8_t *data, zip_uint16_t len)
{
    zip_uint16_t i;

    if (len <= 0)
	return;

    printf("0x");
	
    for (i=0; i<len; i++)
	printf("%02x", data[i]);

    return;    
}

int
main(int argc, char *argv[])
{
    const char *archive;
    struct zip *za, *z_in;
    struct zip_source *zs;
    char buf[100];
    int c, arg, err, flags, idx;

    flags = 0;
    prg = argv[0];

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
    
    arg = optind;

    archive = argv[arg++];

    if (flags == 0)
	flags = ZIP_CREATE;

    if ((za=zip_open(archive, flags, &err)) == NULL) {
	zip_error_to_str(buf, sizeof(buf), err, errno);
	fprintf(stderr, "can't open zip archive '%s': %s\n", archive, buf);
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
		fprintf(stderr, "can't add file '%s': %s\n", argv[arg+1], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "add_dir") == 0 && arg+1 < argc) {
	    /* add directory */
	    if (zip_add_dir(za, argv[arg+1]) < 0) {
		fprintf(stderr, "can't add directory '%s': %s\n", argv[arg+1], zip_strerror(za));
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
		fprintf(stderr, "can't add file '%s': %s\n", argv[arg+1], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 5;
	} else if (strcmp(argv[arg], "add_from_zip") == 0 && arg+5 < argc) {
	    /* add from another zip file */
	    idx = atoi(argv[arg+3]);
	    if ((z_in=zip_open(argv[arg+2], ZIP_CHECKCONS, &err)) == NULL) {
		zip_error_to_str(buf, sizeof(buf), err, errno);
		fprintf(stderr, "can't open source zip archive '%s': %s\n", argv[arg+2], buf);
		err = 1;
		break;
	    }
	    if ((zs=zip_source_zip(za, z_in, idx, 0, atoi(argv[arg+4]), atoi(argv[arg+5]))) == NULL) {
		fprintf(stderr, "error creating file source from '%s' index '%d': %s\n", argv[arg+2], idx, zip_strerror(za));
		zip_close(z_in);
		err = 1;
		break;
	    }
	    if (zip_add(za, argv[arg+1], zs) == -1) {
		fprintf(stderr, "can't add file '%s': %s\n", argv[arg+1], zip_strerror(za));
		zip_source_free(zs);
		zip_close(z_in);
		err = 1;
		break;
	    }
	    arg += 6;
	} else if (strcmp(argv[arg], "count_extra") == 0 && arg+2 < argc) {
	    zip_int16_t count;
	    zip_flags_t ceflags = 0;
	    idx = atoi(argv[arg+1]);
	    ceflags = get_flags(argv[arg+2]);
	    if ((count=zip_file_extra_fields_count(za, idx, ceflags)) < 0) {
		fprintf(stderr, "can't get extra field count for file at index '%d': %s\n", idx, zip_strerror(za));
		err = 1;
		break;
	    } else {
		printf("Extra field count: %d\n", count);
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "count_extra_by_id") == 0 && arg+3 < argc) {
	    zip_int16_t count, eid;
	    zip_flags_t ceflags = 0;
	    idx = atoi(argv[arg+1]);
	    eid = atoi(argv[arg+2]);
	    ceflags = get_flags(argv[arg+3]);
	    if ((count=zip_file_extra_fields_count_by_id(za, idx, eid, ceflags)) < 0) {
		fprintf(stderr, "can't get extra field count for file at index '%d' and for id `%d': %s\n", idx, eid, zip_strerror(za));
		err = 1;
		break;
	    } else {
		printf("Extra field count: %d\n", count);
	    }
	    arg += 4;
	} else if (strcmp(argv[arg], "delete") == 0 && arg+1 < argc) {
	    /* delete */
	    idx = atoi(argv[arg+1]);
	    if (zip_delete(za, idx) < 0) {
		fprintf(stderr, "can't delete file at index '%d': %s\n", idx, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 2;
	} else if (strcmp(argv[arg], "delete_extra") == 0 && arg+1 < argc) {
	    zip_flags_t geflags;
	    zip_uint16_t eid;
	    idx = atoi(argv[arg+1]);
	    eid = atoi(argv[arg+2]);
	    geflags = get_flags(argv[arg+3]);
	    if ((zip_file_extra_field_delete(za, idx, eid, geflags)) < 0) {
		fprintf(stderr, "can't delete extra field data for file at index '%d', extra field id `%d': %s\n", idx, eid, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 4;
	} else if (strcmp(argv[arg], "delete_extra_by_id") == 0 && arg+1 < argc) {
	    zip_flags_t geflags;
	    zip_uint16_t eid, eidx;
	    idx = atoi(argv[arg+1]);
	    eid = atoi(argv[arg+2]);
	    eidx = atoi(argv[arg+3]);
	    geflags = get_flags(argv[arg+4]);
	    if ((zip_file_extra_field_delete_by_id(za, idx, eid, eidx, geflags)) < 0) {
		fprintf(stderr, "can't delete extra field data for file at index '%d', extra field id `%d', extra field idx `%d': %s\n", idx, eid, eidx, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 5;
	} else if (strcmp(argv[arg], "get_archive_comment") == 0) {
	    const char *comment;
	    int len;
	    /* get archive comment */
	    if ((comment=zip_get_archive_comment(za, &len, 0)) == NULL)
		printf("No archive comment\n");
	    else
		printf("Archive comment: %.*s\n", len, comment);
	    arg += 1;
	} else if (strcmp(argv[arg], "get_extra") == 0 && arg+3 < argc) {
	    zip_flags_t geflags;
	    zip_uint16_t id, eidx, eflen;
	    const zip_uint8_t *efdata;
	    /* get extra field data */
	    idx = atoi(argv[arg+1]);
	    eidx = atoi(argv[arg+2]);
	    geflags = get_flags(argv[arg+3]);
	    if ((efdata=zip_file_extra_field_get(za, idx, eidx, &id, &eflen, geflags)) == NULL) {
		fprintf(stderr, "can't get extra field data for file at index %d, extra field %d, flags %u: %s\n", idx, eidx, geflags, zip_strerror(za));
		err = 1;
	    } else {
		printf("Extra field 0x%04x: len %d", id, eflen);
		if (eflen > 0) {
		    printf(", data ");
		    hexdump(efdata, eflen);
		}
		printf("\n");
	    }
	    arg += 4;
	} else if (strcmp(argv[arg], "get_extra_by_id") == 0 && arg+4 < argc) {
	    zip_flags_t geflags;
	    zip_uint16_t eid, eidx, eflen;
	    const zip_uint8_t *efdata;
	    idx = atoi(argv[arg+1]);
	    eid = atoi(argv[arg+2]);
	    eidx = atoi(argv[arg+3]);
	    geflags = get_flags(argv[arg+4]);
	    if ((efdata=zip_file_extra_field_get_by_id(za, idx, eid, eidx, &eflen, geflags)) == NULL) {
		fprintf(stderr, "can't get extra field data for file at index %d, extra field id %d, ef index %d, flags %u: %s\n", idx, eid, eidx, geflags, zip_strerror(za));
		err = 1;
	    } else {
		printf("Extra field 0x%04x: len %d", eid, eflen);
		if (eflen > 0) {
		    printf(", data ");
		    hexdump(efdata, eflen);
		}
		printf("\n");
	    }
	    arg += 5;
	} else if (strcmp(argv[arg], "get_file_comment") == 0 && arg+1 < argc) {
	    const char *comment;
	    int len;
	    /* get file comment */
	    idx = atoi(argv[arg+1]);
	    if ((comment=zip_get_file_comment(za, idx, &len, 0)) == NULL) {
		fprintf(stderr, "can't get comment for '%s': %s\n", zip_get_name(za, idx, 0), zip_strerror(za));
		err = 1;
		break;
	    } else if (len == 0)
		printf("No comment for '%s'\n", zip_get_name(za, idx, 0));
	    else
		printf("File comment for '%s': %.*s\n", zip_get_name(za, idx, 0), len, comment);
	    arg += 2;
	} else if (strcmp(argv[arg], "rename") == 0 && arg+2 < argc) {
	    /* rename */
	    idx = atoi(argv[arg+1]);
	    if (zip_rename(za, idx, argv[arg+2]) < 0) {
		fprintf(stderr, "can't rename file at index '%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
	} else if (strcmp(argv[arg], "set_extra") == 0 && arg+5 < argc) {
	    zip_flags_t geflags;
	    zip_uint16_t eid, eidx;
	    const zip_uint8_t *efdata;
	    idx = atoi(argv[arg+1]);
	    eid = atoi(argv[arg+2]);
	    eidx = atoi(argv[arg+3]);
	    geflags = get_flags(argv[arg+4]);
	    efdata = (zip_uint8_t *)argv[arg+5];
	    if ((zip_file_extra_field_set(za, idx, eid, eidx, efdata, (zip_uint16_t)strlen((const char *)efdata), geflags)) < 0) {
		fprintf(stderr, "can't set extra field data for file at index '%d', extra field id `%d', index `%d': %s\n", idx, eid, eidx, zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 6;
	} else if (strcmp(argv[arg], "set_file_comment") == 0 && arg+2 < argc) {
	    /* set file comment */
	    idx = atoi(argv[arg+1]);
	    if (zip_file_set_comment(za, idx, argv[arg+2], (zip_uint16_t)strlen(argv[arg+2]), 0) < 0) {
		fprintf(stderr, "can't set file comment at index '%d' to `%s': %s\n", idx, argv[arg+2], zip_strerror(za));
		err = 1;
		break;
	    }
	    arg += 3;
        } else if (strcmp(argv[arg], "set_file_compression") == 0 && arg+3 < argc) {
            /* set file compression */
            zip_int32_t method;
            zip_uint32_t flags;
            idx = atoi(argv[arg+1]);
            method = get_compression_method(argv[arg+2]);
            flags = atoi(argv[arg+3]);
            if (zip_set_file_compression(za, idx, method, flags) < 0) {
		fprintf(stderr, "can't set file compression method at index '%d' to `%s', flags `%d': %s\n", idx, argv[arg+2], flags, zip_strerror(za));
		err = 1;
		break;
            }
            arg += 4;
	} else {
	    fprintf(stderr, "unrecognized command '%s', or not enough arguments\n", argv[arg]);
	    err = 1;
	    break;
	}
    }

    if (zip_close(za) == -1) {
	fprintf(stderr, "can't close zip archive '%s': %s\n", archive, zip_strerror(za));
	return 1;
    }

    return err;
}
