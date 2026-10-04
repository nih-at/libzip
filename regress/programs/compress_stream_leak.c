
/*
 compress_stream_leak.c -- exercise freeing an open compression source
 Copyright (C) 2026 Dieter Baron and Thomas Klausner

 This file is part of libzip, a library to manipulate ZIP archives.

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
 ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Opens an archive with one deflated entry from a memory buffer, then
   discards the archive without closing the open file. The compression
   source is freed while its zlib stream is still active; before the
   deallocate fix this leaked the stream state on every iteration,
   which a leak checker reports. */

#include <stdio.h>
#include <string.h>

#include "zip.h"

/* a zip archive containing one deflated file, payload.txt */
static const unsigned char archive[] = {
    80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 167, 188, 67, 93, 210, 145,
    133, 136, 15, 0, 0, 0, 112, 0, 0, 0, 11, 0, 0, 0, 112, 97,
    121, 108, 111, 97, 100, 46, 116, 120, 116, 75, 76, 74, 4, 66, 133, 68,
    44, 20, 23, 54, 65, 74, 229, 0, 80, 75, 1, 2, 20, 3, 20, 0,
    0, 0, 8, 0, 167, 188, 67, 93, 210, 145, 133, 136, 15, 0, 0, 0,
    112, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    128, 1, 0, 0, 0, 0, 112, 97, 121, 108, 111, 97, 100, 46, 116, 120,
    116, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 57, 0, 0,
    0, 56, 0, 0, 0, 0, 0
};

int main(void) {
    char buffer[8192];
    int i;

    for (i = 0; i < 200; i++) {
        zip_error_t error;
        zip_source_t *src;
        zip_t *za;
        zip_file_t *zf;

        zip_error_init(&error);
        src = zip_source_buffer_create(archive, sizeof(archive), 0, &error);
        if (src == NULL) {
            fprintf(stderr, "can't create source on iteration %d: %s\n", i, zip_error_strerror(&error));
            zip_error_fini(&error);
            return 1;
        }
        za = zip_open_from_source(src, 0, &error);
        if (za == NULL) {
            fprintf(stderr, "can't open archive on iteration %d: %s\n", i, zip_error_strerror(&error));
            zip_source_free(src);
            zip_error_fini(&error);
            return 1;
        }
        zip_error_fini(&error);

        zf = zip_fopen(za, "payload.txt", 0);
        if (zf == NULL) {
            fprintf(stderr, "can't open file on iteration %d: %s\n", i, zip_strerror(za));
            zip_discard(za);
            return 1;
        }
        (void)zip_fread(zf, buffer, sizeof(buffer));
        /* deliberately no zip_fclose: this frees the source while it is open */
        zip_discard(za);
    }

    return 0;
}
