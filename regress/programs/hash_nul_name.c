/*
  hash_nul_name.c -- lookups must not stop at embedded NUL bytes in file names
  Copyright (C) 2026 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <info@libzip.org>

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

/* The archive below contains two stored entries with the raw names
   "a\0x" and "a\0y". Opening it without ZIP_CHECKCONS accepts the
   embedded NUL bytes, so name lookups must compare the full raw
   names: no entry is called "a", and adding a file with that name
   must not be rejected as a duplicate. */

#include <stdio.h>
#include <string.h>

#include "zip.h"

static const zip_uint8_t archive_with_nul_names[] = {
    0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x50, 0x4b, 0x03,
    0x04, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x61, 0x00, 0x79, 0x50, 0x4b, 0x01, 0x02, 0x14, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x50, 0x4b, 0x01, 0x02, 0x14,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x21, 0x00, 0x00, 0x00, 0x61, 0x00, 0x79, 0x50, 0x4b, 0x05, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x62, 0x00, 0x00, 0x00,
    0x42, 0x00, 0x00, 0x00, 0x00, 0x00,

};

int main(void) {
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_int64_t index;

    zip_error_init(&error);

    if ((src = zip_source_buffer_create(archive_with_nul_names, sizeof(archive_with_nul_names), 0, &error)) == NULL) {
        fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
        return 1;
    }
    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
        fprintf(stderr, "can't open archive: %s\n", zip_error_strerror(&error));
        return 1;
    }

    if (zip_get_num_entries(za, 0) != 2) {
        fprintf(stderr, "archive has %lld entries, expected 2\n", (long long)zip_get_num_entries(za, 0));
        zip_discard(za);
        return 1;
    }

    if ((index = zip_name_locate(za, "a", 0)) >= 0) {
        fprintf(stderr, "found entry %lld for name \"a\", but no entry has that name\n", (long long)index);
        zip_discard(za);
        return 1;
    }

    if ((index = zip_name_locate(za, "a", ZIP_FL_NOCASE)) >= 0) {
        fprintf(stderr, "found entry %lld for name \"a\" ignoring case, but no entry has that name\n", (long long)index);
        zip_discard(za);
        return 1;
    }

    if ((src = zip_source_buffer(za, "payload", 7, 0)) == NULL || (index = zip_file_add(za, "a", src, 0)) < 0) {
        fprintf(stderr, "can't add file named \"a\": %s\n", zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    if (zip_name_locate(za, "a", 0) != index) {
        fprintf(stderr, "can't find the file named \"a\" again\n");
        zip_discard(za);
        return 1;
    }

    if (zip_get_num_entries(za, 0) != 3) {
        fprintf(stderr, "archive has %lld entries after adding one, expected 3\n", (long long)zip_get_num_entries(za, 0));
        zip_discard(za);
        return 1;
    }

    zip_discard(za);
    zip_error_fini(&error);
    return 0;
}
