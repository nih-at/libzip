/*
  delete_unchange.c -- test case for deleting, unchanging and locating a file
  Copyright (C) 2022 Krzesimir Nowak

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


#include <stdio.h>

#include "zip.h"

int main(int argc, char **argv) {
    const char *archive;
    zip_t *za;
    zip_source_t *zs;
    int err;
    char contents[] = "contents";
    const char* file = "testfile";
    zip_int64_t index;

    if (argc != 2) {
        fprintf(stderr, "usage: %s archive\n", argv[0]);
        return 1;
    }

    archive = argv[1];

    if ((za = zip_open(archive, ZIP_CREATE | ZIP_TRUNCATE, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "can't open zip archive '%s': %s\n", archive, zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    if ((zs = zip_source_buffer(za, contents, sizeof(contents)-1, 0)) == NULL) {
        fprintf(stderr, "can't create zip_source from buffer: %s\n", zip_strerror(za));
        zip_discard (za);
        return 1;
    }

    if (zip_file_add(za, file, zs, 0) < 0) {
        fprintf(stderr, "can't add file '%s': %s\n", file, zip_strerror(za));
        zip_source_free(zs);
        zip_discard(za);
        return 1;
    }

    if (zip_close(za) < 0) {
        fprintf(stderr, "can't close zip archive '%s': %s\n", archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if ((za = zip_open(archive, 0, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "can't open zip archive '%s': %s\n", archive, zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    if ((index = zip_name_locate(za, file, 0)) < 0) {
        fprintf(stderr, "can't locate '%s' in archive '%s': %s\n", file, archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if (zip_delete(za, index) < 0) {
        fprintf(stderr, "can't delete file '%s' from archive '%s': %s\n", file, archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if (zip_unchange(za, index) < 0) {
        fprintf(stderr, "can't unchange file '%s' in archive '%s': %s\n", file, archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if ((index = zip_name_locate(za, file, 0)) < 0) {
        fprintf(stderr, "can't locate '%s' in archive '%s': %s\n", file, archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    /* now just make the archive empty, so it gets removed when closing */
    if (zip_delete(za, index) < 0) {
        fprintf(stderr, "can't delete file '%s' from archive '%s': %s\n", file, archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if (zip_close(za) < 0) {
        fprintf(stderr, "can't close zip archive '%s': %s\n", archive, zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    return 0;
}
