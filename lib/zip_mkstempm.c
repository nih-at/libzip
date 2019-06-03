/*
  zip_mkstempm.c -- mkstemp replacement that accepts a mode argument
  Copyright (C) 2019 Dieter Baron and Thomas Klausner

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

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * create temporary file with same permissions as previous one;
 * or default permissions if there is no previous file
 */
int
_zip_mkstempm(char *path, int mode) {
    int fd;
    char *start, *end, *xs;
    struct stat sbuf;

    srandom(time(NULL));

     /* To guarantee multiple calls generate unique names even if
       the file is not created. 676 different possibilities with 7
       or more X's, 26 with 6 or less. */
    static char xtra[2] = "aa";
    int xcnt = 0;

    end = path + strlen(path);
    xs = end - 1;
    while (xs >= path && *xs == 'X') {
	xcnt++;
	xs--;
    }

    if (xcnt == 0) {
	errno = EINVAL;
	return -1;
    }

    /*
     * check the target directory; if you have six X's and it
     * doesn't exist this runs for a *very* long time.
     */
    for (start = end -1; start >= path; --start) {
	if (*start == '/') {
	    *start = '\0';
	    if (stat(path, &sbuf) < 0) {
		return -1;
	    }
	    if (!S_ISDIR(sbuf.st_mode)) {
		errno = ENOTDIR;
		return -1;
	    }
	    *start = '/';
	    break;
	}
    }

    xs++;

    /* Use at least one from xtra.  Use 2 if more than 6 X's. */
    *(xs++) = xtra[0];
    if (xcnt > 6) {
	*(xs++) = xtra[1];
    }

    start = xs;

    /* update xtra for next call. */
    if (xtra[0] != 'z')
	xtra[0]++;
    else {
	xtra[0] = 'a';
	if (xtra[1] != 'z')
	    xtra[1]++;
	else
	    xtra[1] = 'a';
    }

    for (;;) {
	int value = random();

	xs = start;

	while (xs < end) {
	    *(xs++) = (value % 10) + '0';
	    value /= 10;
	}

	if ((fd = open(path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, mode == -1 ? 0666 : (mode_t)mode)) >= 0) {
	    if (mode != -1) {
		/* open() honors umask(), which we don't want in this case */
		chmod(path, mode);
	    }
	    return fd;
	}
	if (errno != EEXIST) {
	    return -1;
	}
    }
    /*NOTREACHED*/
}
