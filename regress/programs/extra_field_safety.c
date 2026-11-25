/*
  extra_field_safety.c -- test extra field aliasing, bounds, and atomicity
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
#include <stdlib.h>
#include <string.h>

#include "zip.h"

static zip_t *create_memory_archive(zip_source_t **src_out, zip_uint64_t *idx_out) {
    zip_error_t err;
    zip_source_t *src;
    zip_source_t *fs;
    zip_t *za;
    zip_int64_t idx;

    zip_error_init(&err);
    src = zip_source_buffer_create(NULL, 0, 0, &err);
    if (src == NULL) {
        fprintf(stderr, "zip_source_buffer_create failed: %s\n", zip_error_strerror(&err));
        zip_error_fini(&err);
        return NULL;
    }
    zip_source_keep(src);

    za = zip_open_from_source(src, ZIP_TRUNCATE, &err);
    if (za == NULL) {
        fprintf(stderr, "zip_open_from_source failed: %s\n", zip_error_strerror(&err));
        zip_source_free(src);
        zip_error_fini(&err);
        return NULL;
    }
    zip_error_fini(&err);

    fs = zip_source_buffer(za, "payload", 7, 0);
    if (fs == NULL) {
        fprintf(stderr, "zip_source_buffer failed: %s\n", zip_strerror(za));
        zip_discard(za);
        zip_source_free(src);
        return NULL;
    }

    idx = zip_file_add(za, "file.txt", fs, ZIP_FL_ENC_UTF_8);
    if (idx < 0) {
        fprintf(stderr, "zip_file_add failed: %s\n", zip_strerror(za));
        zip_source_free(fs);
        zip_discard(za);
        zip_source_free(src);
        return NULL;
    }

    *src_out = src;
    *idx_out = (zip_uint64_t)idx;
    return za;
}

static int test_aliased_oob_rejected(void) {
    static const zip_uint8_t orig[4] = {0x11, 0x22, 0x33, 0x44};
    zip_source_t *src = NULL;
    zip_uint64_t idx = 0;
    zip_t *za = create_memory_archive(&src, &idx);
    const zip_uint8_t *p;
    zip_uint16_t len = 0;
    int fail = 0;

    if (za == NULL) {
        return 1;
    }

    if (zip_file_extra_field_set(za, idx, 0x1234, ZIP_EXTRA_FIELD_NEW, orig, sizeof(orig), ZIP_FL_LOCAL | ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "setup extra field failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    /* 1. Direct pointer alias with len > ef->size on LOCAL */
    p = zip_file_extra_field_get_by_id(za, idx, 0x1234, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != sizeof(orig)) {
        fprintf(stderr, "unexpected get_by_id local result\n");
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x1234, 0, p, 128, ZIP_FL_LOCAL) == 0 || zip_error_code_zip(zip_get_error(za)) != ZIP_ER_INVAL) {
        fprintf(stderr, "expected ZIP_ER_INVAL for oversized self-alias local set\n");
        fail = 1;
        goto done;
    }

    /* 2. Interior slice alias exceeding remaining length */
    if (zip_file_extra_field_set(za, idx, 0x1234, 0, p + 1, sizeof(orig), ZIP_FL_LOCAL) == 0 || zip_error_code_zip(zip_get_error(za)) != ZIP_ER_INVAL) {
        fprintf(stderr, "expected ZIP_ER_INVAL for oversized slice-alias set\n");
        fail = 1;
        goto done;
    }

    /* 3. Direct pointer alias with len > ef->size on CENTRAL */
    p = zip_file_extra_field_get_by_id(za, idx, 0x1234, 0, &len, ZIP_FL_CENTRAL);
    if (p == NULL || len != sizeof(orig)) {
        fprintf(stderr, "unexpected get_by_id central result\n");
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x1234, 0, p, 64, ZIP_FL_CENTRAL) == 0 || zip_error_code_zip(zip_get_error(za)) != ZIP_ER_INVAL) {
        fprintf(stderr, "expected ZIP_ER_INVAL for oversized self-alias central set\n");
        fail = 1;
        goto done;
    }

    /* 4. Zero-length empty sentinel returned by get_by_id must be rejected with len > 0 */
    if (zip_file_extra_field_set(za, idx, 0x2222, ZIP_EXTRA_FIELD_NEW, NULL, 0, ZIP_FL_LOCAL) < 0) {
        fprintf(stderr, "setup zero-length extra field failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }
    p = zip_file_extra_field_get_by_id(za, idx, 0x2222, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != 0) {
        fprintf(stderr, "unexpected zero-length get_by_id result\n");
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x2222, 0, p, 16, ZIP_FL_LOCAL) == 0 || zip_error_code_zip(zip_get_error(za)) != ZIP_ER_INVAL) {
        fprintf(stderr, "expected ZIP_ER_INVAL for empty sentinel with positive length\n");
        fail = 1;
        goto done;
    }

    if (zip_close(za) < 0) {
        fprintf(stderr, "zip_close failed: %s\n", zip_strerror(za));
        fail = 1;
        zip_discard(za);
        za = NULL;
    }
    else {
        za = NULL;
    }

done:
    if (za != NULL) {
        zip_discard(za);
    }
    zip_source_free(src);
    return fail;
}

static int test_cross_list_slice_and_shrink(void) {
    static const zip_uint8_t orig[4] = {0x0a, 0x14, 0x1e, 0x28};
    static const zip_uint8_t expected_slice[2] = {0x14, 0x1e};
    zip_source_t *src = NULL;
    zip_uint64_t idx = 0;
    zip_t *za = create_memory_archive(&src, &idx);
    const zip_uint8_t *p;
    zip_uint16_t len = 0;
    zip_error_t err;
    int fail = 0;

    if (za == NULL) {
        return 1;
    }

    if (zip_file_extra_field_set(za, idx, 0x3333, ZIP_EXTRA_FIELD_NEW, orig, sizeof(orig), ZIP_FL_LOCAL | ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "setup cross-list extra field failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    /* Pass an in-bounds slice of LOCAL's buffer to update BOTH LOCAL and CENTRAL */
    p = zip_file_extra_field_get_by_id(za, idx, 0x3333, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != sizeof(orig)) {
        fprintf(stderr, "unexpected get_by_id result\n");
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x3333, 0, p + 1, 2, ZIP_FL_LOCAL | ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "valid slice set across BOTH failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    /* Shrink LOCAL in-place from 2 -> 1, then verify growing back to 2 is rejected */
    p = zip_file_extra_field_get_by_id(za, idx, 0x3333, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != 2 || memcmp(p, expected_slice, 2) != 0) {
        fprintf(stderr, "unexpected slice content in local\n");
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x3333, 0, p, 1, ZIP_FL_LOCAL) < 0) {
        fprintf(stderr, "valid in-place shrink failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }
    if (zip_file_extra_field_set(za, idx, 0x3333, 0, p, 2, ZIP_FL_LOCAL) == 0) {
        fprintf(stderr, "expected regrowing shrunk alias to be rejected\n");
        fail = 1;
        goto done;
    }

    /* Restore central-matching slice for close check */
    if (zip_file_extra_field_set(za, idx, 0x3333, 0, expected_slice, 2, ZIP_FL_LOCAL) < 0) {
        fprintf(stderr, "restore local slice failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    if (zip_close(za) < 0) {
        fprintf(stderr, "zip_close failed: %s\n", zip_strerror(za));
        zip_discard(za);
        za = NULL;
        fail = 1;
        goto done;
    }
    za = NULL;

    /* Reopen and verify persisted bytes */
    zip_source_keep(src);
    zip_error_init(&err);
    za = zip_open_from_source(src, ZIP_RDONLY | ZIP_CHECKCONS, &err);
    if (za == NULL) {
        fprintf(stderr, "reopen failed: %s\n", zip_error_strerror(&err));
        zip_error_fini(&err);
        zip_source_free(src);
        fail = 1;
        goto done;
    }
    zip_error_fini(&err);

    p = zip_file_extra_field_get_by_id(za, 0, 0x3333, 0, &len, ZIP_FL_CENTRAL);
    if (p == NULL || len != 2 || memcmp(p, expected_slice, 2) != 0) {
        fprintf(stderr, "reopened central extra field mismatch\n");
        fail = 1;
    }
    p = zip_file_extra_field_get_by_id(za, 0, 0x3333, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != 2 || memcmp(p, expected_slice, 2) != 0) {
        fprintf(stderr, "reopened local extra field mismatch\n");
        fail = 1;
    }

done:
    if (za != NULL) {
        zip_discard(za);
    }
    zip_source_free(src);
    return fail;
}

static int test_both_flags_atomicity(void) {
    static const zip_uint8_t orig[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    zip_uint8_t filler[65500];
    zip_uint8_t replacement[100];
    zip_source_t *src = NULL;
    zip_uint64_t idx = 0;
    zip_t *za = create_memory_archive(&src, &idx);
    const zip_uint8_t *p;
    zip_uint16_t len = 0;
    int fail = 0;

    if (za == NULL) {
        return 1;
    }

    memset(filler, 0x55, sizeof(filler));
    memset(replacement, 0x77, sizeof(replacement));

    if (zip_file_extra_field_set(za, idx, 0x4444, ZIP_EXTRA_FIELD_NEW, orig, sizeof(orig), ZIP_FL_LOCAL | ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "setup 0x4444 failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }
    /* Fill CENTRAL close to 65535 so replacement fits LOCAL but overflows CENTRAL */
    if (zip_file_extra_field_set(za, idx, 0x4445, ZIP_EXTRA_FIELD_NEW, filler, sizeof(filler), ZIP_FL_CENTRAL) < 0) {
        fprintf(stderr, "setup filler failed: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    if (zip_file_extra_field_set(za, idx, 0x4444, 0, replacement, sizeof(replacement), ZIP_FL_LOCAL | ZIP_FL_CENTRAL) == 0) {
        fprintf(stderr, "expected ZIP_ER_EF_TOO_LARGE when central overflows\n");
        fail = 1;
        goto done;
    }
    if (zip_error_code_zip(zip_get_error(za)) != ZIP_ER_EF_TOO_LARGE) {
        fprintf(stderr, "unexpected error code: %s\n", zip_strerror(za));
        fail = 1;
        goto done;
    }

    p = zip_file_extra_field_get_by_id(za, idx, 0x4444, 0, &len, ZIP_FL_LOCAL);
    if (p == NULL || len != sizeof(orig) || memcmp(p, orig, sizeof(orig)) != 0) {
        fprintf(stderr, "local extra field was partially mutated on failed BOTH set\n");
        fail = 1;
        goto done;
    }

done:
    if (za != NULL) {
        zip_discard(za);
    }
    zip_source_free(src);
    return fail;
}

int main(void) {
    int fail = 0;

    fail += test_aliased_oob_rejected();
    fail += test_cross_list_slice_and_shrink();
    fail += test_both_flags_atomicity();

    return fail ? 1 : 0;
}
