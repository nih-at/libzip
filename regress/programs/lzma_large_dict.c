/*
  lzma_large_dict.c -- regression test for oversized LZMA dictionary headers
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

#include "config.h"

#include <stdio.h>

#include "zip.h"

static const zip_uint8_t lzma_large_dict_archive[] = {
    0x50, 0x4b, 0x03, 0x04, 0x3f, 0x00, 0x02, 0x00, 0x0e, 0x00, 0x89, 0x8a, 0x36, 0x4f, 0x28, 0xe2, 0xde, 0xa0, 0x21, 0x00, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x61, 0x62, 0x61, 0x63, 0x2d, 0x72, 0x65, 0x70, 0x65, 0x61, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x09,
    0x14, 0x05, 0x00, 0x5d, 0x00, 0x00, 0x04, 0xa1, 0x00, 0x00, 0x00, 0xbd, 0xa0, 0xa3, 0x19, 0xd7, 0x9c, 0xf2, 0xec, 0x93, 0x6b, 0xfe, 0x83,
    0x68, 0x26, 0x2f, 0xff, 0xff, 0x33, 0x14, 0x00, 0x00, 0x50, 0x4b, 0x01, 0x02, 0x3f, 0x00, 0x3f, 0x00, 0x02, 0x00, 0x0e, 0x00, 0x89, 0x8a,
    0x36, 0x4f, 0x28, 0xe2, 0xde, 0xa0, 0x21, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x62, 0x61, 0x63, 0x2d, 0x72, 0x65, 0x70, 0x65, 0x61, 0x74, 0x2e, 0x74, 0x78,
    0x74, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00,
};

int main(void) {
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_file_t *zf;
    zip_error_t *zf_error;
    char buf[32];
    zip_int64_t n;

    zip_error_init(&error);
    src = zip_source_buffer_create(lzma_large_dict_archive, sizeof(lzma_large_dict_archive), 0, &error);
    if (src == NULL) {
        fprintf(stderr, "zip_source_buffer_create failed: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    za = zip_open_from_source(src, 0, &error);
    if (za == NULL) {
        fprintf(stderr, "zip_open_from_source failed: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        zip_error_fini(&error);
        return 1;
    }
    zip_error_fini(&error);

    zf = zip_fopen_index(za, 0, 0);
    if (zf == NULL) {
        fprintf(stderr, "zip_fopen_index failed: %s\n", zip_strerror(za));
        zip_discard(za);
        return 1;
    }

    n = zip_fread(zf, buf, sizeof(buf));
    if (n >= 0) {
        fprintf(stderr, "zip_fread unexpectedly returned %lld bytes\n", (long long)n);
        (void)zip_fclose(zf);
        zip_discard(za);
        return 1;
    }

    zf_error = zip_file_get_error(zf);
    if (zip_error_code_zip(zf_error) != ZIP_ER_COMPRESSED_DATA) {
        fprintf(stderr, "zip_fread failed with unexpected error: %s\n", zip_file_strerror(zf));
        (void)zip_fclose(zf);
        zip_discard(za);
        return 1;
    }

    (void)zip_fclose(zf);
    zip_discard(za);
    return 0;
}
