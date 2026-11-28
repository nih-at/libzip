/*
  dirent_dd_seek.c -- regress Zip64 data descriptor seek bounds
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

static void put64(zip_uint8_t *p, zip_uint64_t v) {
    put32(p, (zip_uint32_t)(v & 0xffffffffu));
    put32(p + 4, (zip_uint32_t)(v >> 32));
}

static int test_dd_seek_overflow(zip_uint64_t comp_size) {
    zip_uint8_t buf[256];
    zip_uint32_t cd_off, cd_size, pos;
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;

    memset(buf, 0, sizeof(buf));
    pos = 0;

    /* Local file header (30 bytes) + 1-byte name "a" */
    buf[pos + 0] = 'P';
    buf[pos + 1] = 'K';
    buf[pos + 2] = 3;
    buf[pos + 3] = 4;
    put16(buf + pos + 4, 45);      /* version needed */
    put16(buf + pos + 6, 0x0008);  /* ZIP_GPBF_DATA_DESCRIPTOR */
    put16(buf + pos + 8, 0);       /* STORE */
    put16(buf + pos + 26, 1);      /* filename length */
    buf[pos + 30] = 'a';
    pos += 31;

    /* 20-byte Zip64 data descriptor placeholder */
    pos += 20;

    /* Central directory entry (46 bytes) + 1-byte name "a" + 20-byte Zip64 EF */
    cd_off = pos;
    buf[pos + 0] = 'P';
    buf[pos + 1] = 'K';
    buf[pos + 2] = 1;
    buf[pos + 3] = 2;
    put16(buf + pos + 4, 45);           /* version made by */
    put16(buf + pos + 6, 45);           /* version needed */
    put16(buf + pos + 8, 0x0008);       /* ZIP_GPBF_DATA_DESCRIPTOR */
    put16(buf + pos + 10, 0);           /* STORE */
    put32(buf + pos + 20, 0xffffffffu); /* comp_size -> Zip64 */
    put32(buf + pos + 24, 0xffffffffu); /* uncomp_size -> Zip64 */
    put16(buf + pos + 28, 1);           /* filename length */
    put16(buf + pos + 30, 20);          /* extra field length */
    put32(buf + pos + 42, 0);           /* local header offset */
    buf[pos + 46] = 'a';
    put16(buf + pos + 47, 0x0001);      /* Zip64 EF tag */
    put16(buf + pos + 49, 16);          /* Zip64 EF size */
    put64(buf + pos + 51, comp_size);   /* uncomp_size */
    put64(buf + pos + 59, comp_size);   /* comp_size */
    pos += 67;
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

    za = zip_open_from_source(src, ZIP_CHECKCONS, &error);
    if (za != NULL) {
        fprintf(stderr, "unexpectedly opened archive with central_compressed_size > ZIP_INT64_MAX\n");
        zip_discard(za);
        zip_error_fini(&error);
        return -1;
    }

    if (zip_error_code_zip(&error) != ZIP_ER_SEEK || zip_error_code_system(&error) != EFBIG) {
        fprintf(stderr, "expected ZIP_ER_SEEK/EFBIG, got %d/%d (%s)\n", zip_error_code_zip(&error), zip_error_code_system(&error), zip_error_strerror(&error));
        zip_source_free(src);
        zip_error_fini(&error);
        return -1;
    }

    zip_source_free(src);
    zip_error_fini(&error);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [ignored]\n", argv[0]);
        return 1;
    }

    if (test_dd_seek_overflow((zip_uint64_t)-16) < 0) {
        return 1;
    }
    if (test_dd_seek_overflow((zip_uint64_t)ZIP_INT64_MAX + 1) < 0) {
        return 1;
    }

    return 0;
}
