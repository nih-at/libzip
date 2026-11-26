/*
  extra_field_safety.c -- regress extra-field seek error propagation and atomic ZIP_EF_BOTH set
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

static void put16(zip_uint8_t *p, zip_uint16_t v) {
    p[0] = (zip_uint8_t)(v & 0xff);
    p[1] = (zip_uint8_t)((v >> 8) & 0xff);
}

static void put32(zip_uint8_t *p, zip_uint32_t v) {
    p[0] = (zip_uint8_t)(v & 0xff);
    p[1] = (zip_uint8_t)((v >> 8) & 0xff);
    p[2] = (zip_uint8_t)((v >> 16) & 0xff);
    p[3] = (zip_uint8_t)((v >> 24) & 0xff);
}

static int test_local_ef_seek_error(void) {
    zip_uint8_t buf[128];
    zip_uint32_t cd_off, cd_size, pos;
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_int16_t cnt;

    memset(buf, 0, sizeof(buf));
    pos = 0;

    /* Local header with corrupt fname_len=0xffff and ef_len=4 */
    buf[pos + 0] = 'P';
    buf[pos + 1] = 'K';
    buf[pos + 2] = 3;
    buf[pos + 3] = 4;
    put16(buf + pos + 4, 20);
    put16(buf + pos + 26, 0xffff); /* fname_len past end of buffer */
    put16(buf + pos + 28, 4);      /* ef_len > 0 */
    pos += 30;

    /* Central directory entry (46 bytes) + 1-byte name "a" */
    cd_off = pos;
    buf[pos + 0] = 'P';
    buf[pos + 1] = 'K';
    buf[pos + 2] = 1;
    buf[pos + 3] = 2;
    put16(buf + pos + 4, 20);
    put16(buf + pos + 6, 20);
    put16(buf + pos + 28, 1);
    put32(buf + pos + 42, 0);
    buf[pos + 46] = 'a';
    pos += 47;
    cd_size = pos - cd_off;

    /* EOCD (22 bytes) */
    buf[pos + 0] = 'P';
    buf[pos + 1] = 'K';
    buf[pos + 2] = 5;
    buf[pos + 3] = 6;
    put16(buf + pos + 8, 1);
    put16(buf + pos + 10, 1);
    put32(buf + pos + 12, cd_size);
    put32(buf + pos + 16, cd_off);
    pos += 22;

    zip_error_init(&error);
    if ((src = zip_source_buffer_create(buf, pos, 0, &error)) == NULL) {
        fprintf(stderr, "can't create buffer source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return -1;
    }

    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
        fprintf(stderr, "can't open archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        zip_error_fini(&error);
        return -1;
    }
    zip_error_fini(&error);

    errno = EACCES;
    cnt = zip_file_extra_fields_count(za, 0, ZIP_FL_LOCAL);
    if (cnt != -1) {
        fprintf(stderr, "expected failure reading local extra fields, got %d\n", (int)cnt);
        zip_discard(za);
        return -1;
    }

    if (zip_error_code_zip(zip_get_error(za)) != ZIP_ER_INVAL || zip_error_code_system(zip_get_error(za)) != 0) {
        fprintf(stderr, "expected ZIP_ER_INVAL/0, got %d/%d (%s)\n", zip_error_code_zip(zip_get_error(za)), zip_error_code_system(zip_get_error(za)), zip_strerror(za));
        zip_discard(za);
        return -1;
    }

    zip_discard(za);
    return 0;
}

static int test_atomic_set_both(void) {
    zip_error_t error;
    zip_source_t *src, *data_src;
    zip_t *za;
    zip_uint8_t *big_ef;
    zip_uint8_t small_ef[32];
    zip_int64_t idx;
    zip_int16_t local_count, central_count;

    big_ef = (zip_uint8_t *)calloc(1, 65500);
    if (big_ef == NULL) {
        fprintf(stderr, "out of memory\n");
        return -1;
    }
    memset(small_ef, 0x42, sizeof(small_ef));

    zip_error_init(&error);
    if ((src = zip_source_buffer_create(NULL, 0, 0, &error)) == NULL) {
        fprintf(stderr, "can't create archive buffer source: %s\n", zip_error_strerror(&error));
        free(big_ef);
        zip_error_fini(&error);
        return -1;
    }

    if ((za = zip_open_from_source(src, ZIP_CREATE | ZIP_TRUNCATE, &error)) == NULL) {
        fprintf(stderr, "can't create archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        free(big_ef);
        zip_error_fini(&error);
        return -1;
    }
    zip_error_fini(&error);

    if ((data_src = zip_source_buffer(za, "hello", 5, 0)) == NULL) {
        fprintf(stderr, "can't create entry source: %s\n", zip_strerror(za));
        zip_discard(za);
        free(big_ef);
        return -1;
    }
    if ((idx = zip_file_add(za, "test.txt", data_src, 0)) < 0) {
        fprintf(stderr, "can't add entry: %s\n", zip_strerror(za));
        zip_source_free(data_src);
        zip_discard(za);
        free(big_ef);
        return -1;
    }

    /* Fill central extra fields near ZIP_UINT16_MAX so adding 32+4 bytes fails only on central */
    if (zip_file_extra_field_set(za, (zip_uint64_t)idx, 0x4321, ZIP_EXTRA_FIELD_NEW, big_ef, 65500, ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "can't set initial central extra field: %s\n", zip_strerror(za));
        zip_discard(za);
        free(big_ef);
        return -1;
    }
    free(big_ef);

    if (zip_file_extra_field_set(za, (zip_uint64_t)idx, 0x4322, ZIP_EXTRA_FIELD_NEW, small_ef, sizeof(small_ef), ZIP_FL_LOCAL | ZIP_FL_CENTRAL) == 0) {
        fprintf(stderr, "expected ZIP_ER_EF_TOO_LARGE when setting both extra fields\n");
        zip_discard(za);
        return -1;
    }

    if (zip_error_code_zip(zip_get_error(za)) != ZIP_ER_EF_TOO_LARGE) {
        fprintf(stderr, "expected ZIP_ER_EF_TOO_LARGE, got %d (%s)\n", zip_error_code_zip(zip_get_error(za)), zip_strerror(za));
        zip_discard(za);
        return -1;
    }

    local_count = zip_file_extra_fields_count(za, (zip_uint64_t)idx, ZIP_FL_LOCAL);
    central_count = zip_file_extra_fields_count(za, (zip_uint64_t)idx, ZIP_FL_CENTRAL);
    if (local_count != 0 || central_count != 1) {
        fprintf(stderr, "expected atomic failure (local=0, central=1), got (local=%d, central=%d)\n", (int)local_count, (int)central_count);
        zip_discard(za);
        return -1;
    }

    zip_discard(za);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [ignored]\n", argv[0]);
        return 1;
    }

    if (test_local_ef_seek_error() < 0) {
        return 1;
    }
    if (test_atomic_set_both() < 0) {
        return 1;
    }

    return 0;
}
