/*
  $NiH$

  zipmerge.c -- merge zip archives
  Copyright (C) 2004 Dieter Baron and Thomas Klausner

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



#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "zip.h"



char *prg;

char *usage = "usage: %s [-hVDiI] target-zip zip...\n";

char help_head[] = PACKAGE " by Dieter Baron and Thomas Klausner\n\n";

char help[] = "\n\
  -h       display this help message\n\
  -V       display version number\n\
  -D       ignore directory component in file names\n\
  -i       ask before overwriting files\n\
  -I       ignore case in file names\n\
\n\
Report bugs to <nih@giga.or.at>.\n";

char version_string[] = PACKAGE " " VERSION "\n\
Copyright (C) 2003 Dieter Baron and Thomas Klausner\n\
" PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n\
" PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

#define OPTIONS "hVDiI"

int confirm;
int name_flags;

static int confirm_replace(const char *, const char *, const char *);
static int merge_zip(struct zip *zt, const char *, const char *);



int
main(int argc, char *argv[])
{
    struct zip *zt;
    int c, err;
    char errstr[1024], *tname;

    prg = argv[0];

    confirm = 0;
    name_flags = 0;

    while ((c=getopt(argc, argv, OPTIONS)) != -1) {
	switch (c) {
	case 'D':
	    name_flags |= ZIP_FL_NODIR;
	    break;
	case 'i':
	    confirm = 1;
	    break;
	case 'I':
	    name_flags |= ZIP_FL_NOCASE;
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

    if (argc < optind+2) {
	fprintf(stderr, usage, prg);
	exit(2);
    }

    tname = argv[optind++];
    if ((zt=zip_open(tname, ZIP_CREATE, &err)) == NULL) {
	zip_error_str(errstr, sizeof(errstr), err, errno);
	fprintf(stderr, "%s: cannot open zip archive `%s': %s\n",
		prg, tname, errstr);
	exit(1);
    }

    while (optind<argc) {
	if (merge_zip(zt, tname, argv[optind++]) < 0)
	    exit(1);
    }

    if (zip_close(zt) < 0) {
	fprintf(stderr, "%s: cannot write zip archive `%s': %s\n",
		prg, tname, zip_strerror(zt));
	exit(1);
    }

    exit(0);
}



static int
confirm_replace(const char *tname, const char *sname, const char *fname)
{
    char line[1024];

    printf("replace `%s' in `%s' from `%s'? ",
	   fname, tname, sname);
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL)
	return 0;

    if (tolower(line[0]) == 'y')
	return 1;

    return 0;
}



static int
merge_zip(struct zip *zt, const char *tname, const char *sname)
{
    struct zip *zs;
    int i, idx, err;
    char errstr[1024];
    const char *fname;
    
    if ((zs=zip_open(sname, 0, &err)) == NULL) {
	zip_error_str(errstr, sizeof(errstr), err, errno);
	fprintf(stderr, "%s: cannot open zip archive `%s': %s\n",
		prg, sname, errstr);
	return -1;
    }

    for (i=0; i<zip_get_num_files(zs); i++) {
	fname = zip_get_name(zs, i);

	if ((idx=zip_name_locate(zt, fname, name_flags)) != -1) {
	    if (!confirm || confirm_replace(tname, sname, fname)) {
		if (zip_replace_zip(zt, idx, zs, i, 0, 0, 0) < 0) {
		    fprintf(stderr,
			    "%s: cannot replace `%s' in `%s': %s\n",
			    prg, fname, tname, zip_strerror(zt));
		    return -1;
		}
	    }
	}
	else {
	    if (zip_add_zip(zt, fname, zs, i, 0, 0, 0) < 0) {
		fprintf(stderr,
			"%s: cannot add `%s' to `%s': %s\n",
			prg, fname, tname, zip_strerror(zt));
		return -1;
	    }
	}
    }

    return 0;
}
