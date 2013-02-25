/*
  stat_index.c -- stat entry in archive and print details
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


#include "config.h"

#include <errno.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>


#ifndef HAVE_GETOPT
#include "getopt.h"
#endif

#include "zip.h"
#include "compat.h"

const char *prg;

const char *usage = "usage: %s [-grs] file index [index ...]\n";



int
main(int argc, char *argv[])
{
    const char *fname;
    char buf[100];
    struct zip *z;
    int c, flags, ze;
    struct zip_stat sb;
    int index;

    flags = 0;
    prg = argv[0];

    while ((c=getopt(argc, argv, "grs")) != -1) {
	switch (c) {
	case 'g':
	    flags = ZIP_FL_ENC_GUESS;
	    break;
	case 'r':
	    flags = ZIP_FL_ENC_RAW;
	    break;
	case 's':
	    flags = ZIP_FL_ENC_STRICT;
	    break;

	default:
	    fprintf(stderr, usage, prg);
	    return 1;
	}
    }
    if (argc < optind+2) {
	fprintf(stderr, usage, prg);
	return 1;
    }

    fname = argv[optind++];
    errno = 0;

    if ((z=zip_open(fname, flags, &ze)) == NULL) {
	zip_error_to_str(buf, sizeof(buf), ze, errno);
	fprintf(stderr, "%s: can't open zip archive `%s': %s\n", prg,
		fname, buf);
	return 1;
    }

    while (optind < argc) {
        index = atoi(argv[optind++]);

	if (zip_stat_index(z, index, flags, &sb) < 0) {
	    fprintf(stderr, "%s: zip_stat_index failed on `%d' failed: %s\n",
		    prg, index, zip_strerror(z));
	    return 1;
	}

	if (sb.valid & ZIP_STAT_NAME)
	    printf("name: `%s'\n", sb.name);
	if (sb.valid & ZIP_STAT_INDEX)
	    printf("index: `%"PRIu64"'\n", sb.index);
	if (sb.valid & ZIP_STAT_SIZE)
	    printf("size: `%"PRIu64"'\n", sb.size);
	if (sb.valid & ZIP_STAT_COMP_SIZE)
	    printf("compressed size: `%"PRIu64"'\n", sb.comp_size);
	if (sb.valid & ZIP_STAT_MTIME) {
	    struct tm *tpm;
	    tpm = gmtime(&sb.mtime);
	    strftime(buf, sizeof(buf), "%a %b %d %T %Y", tpm);
	    printf("mtime: `%s'\n", buf);
	}
	if (sb.valid & ZIP_STAT_CRC)
	    printf("crc: `%0x'\n", sb.crc);
	if (sb.valid & ZIP_STAT_COMP_METHOD)
	    printf("compression method: `%d'\n", sb.comp_method);
	if (sb.valid & ZIP_STAT_ENCRYPTION_METHOD)
	    printf("encryption method: `%d'\n", sb.encryption_method);
	if (sb.valid & ZIP_STAT_FLAGS)
	    printf("flags: `%ld'\n", (long)sb.flags);
	printf("\n");
    }

    if (zip_close(z) == -1) {
	fprintf(stderr, "%s: can't close zip archive `%s': %s\n", prg, fname, zip_strerror(z));
	return 1;
    }

    return 0;
}
