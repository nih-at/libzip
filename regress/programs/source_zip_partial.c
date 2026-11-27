/*
  source_zip_partial.c -- verify partial zip source bounds and comp_size checks
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

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "zip.h"

#include "cmocka_utilities.h"

static const char payload_data[] = "0123456789";

enum fixture_kind {
    FIXTURE_PENDING_NO_COMP_SIZE,
    FIXTURE_PENDING_SIZE_ONLY,
    FIXTURE_PENDING_STORED_COMP_SIZE,
    FIXTURE_ARCHIVE_STORED,
    FIXTURE_ARCHIVE_STORED_MISMATCH,
    FIXTURE_ARCHIVE_EMPTY
};

typedef enum fixture_kind fixture_kind_t;

struct stream_ctx {
    size_t offset;
    zip_uint64_t stat_flags;
    zip_uint64_t reported_size;
    zip_uint64_t reported_comp_size;
};

typedef struct stream_ctx stream_ctx_t;

struct test_case {
    fixture_kind_t fixture;
    zip_uint64_t start;
    zip_int64_t length;
    int expected_error;
    const char *expected_data;
    size_t expected_length;
};

typedef struct test_case test_case_t;

static zip_int64_t stream_source_cb(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    stream_ctx_t *ctx = (stream_ctx_t *)userdata;
    size_t total = sizeof(payload_data) - 1;

    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        ctx->offset = 0;
        return 0;

    case ZIP_SOURCE_READ: {
        size_t avail = total - ctx->offset;
        size_t n = len < (zip_uint64_t)avail ? (size_t)len : avail;

        if (n > 0) {
            memcpy(data, payload_data + ctx->offset, n);
            ctx->offset += n;
        }
        return (zip_int64_t)n;
    }

    case ZIP_SOURCE_CLOSE:
    case ZIP_SOURCE_FREE:
        return 0;

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st = (zip_stat_t *)data;

        zip_stat_init(st);
        st->valid = ctx->stat_flags;
        if (ctx->stat_flags & ZIP_STAT_SIZE) {
            st->size = ctx->reported_size;
        }
        if (ctx->stat_flags & ZIP_STAT_COMP_SIZE) {
            st->comp_size = ctx->reported_comp_size;
        }
        if (ctx->stat_flags & ZIP_STAT_COMP_METHOD) {
            st->comp_method = ZIP_CM_STORE;
        }
        return (zip_int64_t)sizeof(*st);
    }

    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_READABLE | zip_source_make_command_bitmap(ZIP_SOURCE_SUPPORTS_REOPEN, -1);

    default:
        return -1;
    }
}

static size_t build_stored_archive(zip_uint8_t *out, size_t cap, const char *content, size_t content_len, bool inflate_cdir_uncomp_size) {
    zip_error_t err;
    zip_source_t *archive_src;
    zip_source_t *entry_src;
    zip_t *za;
    zip_int64_t idx;
    zip_int64_t got;
    size_t archive_len;
    size_t i;

    zip_error_init(&err);
    archive_src = zip_source_buffer_create(NULL, 0, 0, &err);
    assert_non_null(archive_src);
    zip_source_keep(archive_src);

    za = zip_open_from_source(archive_src, ZIP_CREATE | ZIP_TRUNCATE, &err);
    assert_non_null(za);

    entry_src = zip_source_buffer(za, content, content_len, 0);
    assert_non_null(entry_src);
    idx = zip_file_add(za, "entry.bin", entry_src, 0);
    assert_true(idx == 0);
    assert_int_equal(zip_set_file_compression(za, (zip_uint64_t)idx, ZIP_CM_STORE, 0), 0);
    assert_int_equal(zip_close(za), 0);

    assert_int_equal(zip_source_open(archive_src), 0);
    got = zip_source_read(archive_src, out, cap);
    assert_true(got > 0 && (size_t)got < cap);
    archive_len = (size_t)got;
    assert_int_equal(zip_source_close(archive_src), 0);
    zip_source_free(archive_src);
    zip_error_fini(&err);

    if (inflate_cdir_uncomp_size) {
        bool patched = false;

        for (i = 0; i + 46 <= archive_len; i++) {
            if (memcmp(out + i, "PK\1\2", 4) == 0) {
                assert_int_equal(out[i + 20], (int)content_len);
                assert_int_equal(out[i + 24], (int)content_len);
                out[i + 24] = (zip_uint8_t)(content_len + 2);
                patched = true;
                break;
            }
        }
        assert_true(patched);
    }

    return archive_len;
}

static zip_t *open_fixture_archive(fixture_kind_t fixture, stream_ctx_t *sctx, zip_uint8_t *archive_buf, size_t archive_cap) {
    zip_error_t err;
    zip_source_t *backing;
    zip_source_t *entry_src;
    zip_t *za;
    zip_int64_t idx;
    size_t raw_len;

    memset(sctx, 0, sizeof(*sctx));
    zip_error_init(&err);

    if (fixture == FIXTURE_ARCHIVE_STORED || fixture == FIXTURE_ARCHIVE_STORED_MISMATCH || fixture == FIXTURE_ARCHIVE_EMPTY) {
        if (fixture == FIXTURE_ARCHIVE_EMPTY) {
            raw_len = build_stored_archive(archive_buf, archive_cap, "", 0, false);
        }
        else {
            raw_len = build_stored_archive(archive_buf, archive_cap, payload_data, sizeof(payload_data) - 1, fixture == FIXTURE_ARCHIVE_STORED_MISMATCH);
        }

        backing = zip_source_buffer_create(archive_buf, raw_len, 0, &err);
        assert_non_null(backing);
        za = zip_open_from_source(backing, 0, &err);
        assert_non_null(za);
        zip_error_fini(&err);
        return za;
    }

    backing = zip_source_buffer_create(NULL, 0, 0, &err);
    assert_non_null(backing);
    za = zip_open_from_source(backing, ZIP_CREATE, &err);
    assert_non_null(za);

    if (fixture == FIXTURE_PENDING_SIZE_ONLY) {
        sctx->stat_flags = ZIP_STAT_SIZE;
        sctx->reported_size = sizeof(payload_data) - 1;
    }
    else if (fixture == FIXTURE_PENDING_STORED_COMP_SIZE) {
        sctx->stat_flags = ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD;
        sctx->reported_comp_size = sizeof(payload_data) - 1;
    }

    entry_src = zip_source_function_create(stream_source_cb, sctx, &err);
    assert_non_null(entry_src);
    idx = zip_file_add(za, "pending.bin", entry_src, 0);
    assert_true(idx == 0);

    if (fixture == FIXTURE_PENDING_STORED_COMP_SIZE) {
        assert_int_equal(zip_set_file_compression(za, (zip_uint64_t)idx, ZIP_CM_STORE, 0), 0);
    }

    zip_error_fini(&err);
    return za;
}

static test_case_t tests[] = {
    /* 1. Reopenable source without ZIP_STAT_COMP_SIZE. */
    {FIXTURE_PENDING_NO_COMP_SIZE, 0, -1, 0, "0123456789", 10},
    {FIXTURE_PENDING_NO_COMP_SIZE, 3, 4, 0, "3456", 4},
    {FIXTURE_PENDING_NO_COMP_SIZE, 3, -1, 0, "3456789", 7},
    {FIXTURE_PENDING_SIZE_ONLY, 2, 4, 0, "2345", 4},

    /* 2. Stored source with valid ZIP_STAT_COMP_SIZE: exact boundary & empty ownership. */
    {FIXTURE_PENDING_STORED_COMP_SIZE, 6, 4, 0, "6789", 4},
    {FIXTURE_PENDING_STORED_COMP_SIZE, 10, 0, 0, "", 0},
    {FIXTURE_ARCHIVE_STORED, 6, 4, 0, "6789", 4},
    {FIXTURE_ARCHIVE_STORED, 10, 0, 0, "", 0},
    {FIXTURE_ARCHIVE_EMPTY, 0, -1, 0, "", 0},
    {FIXTURE_ARCHIVE_EMPTY, 0, 0, 0, "", 0},

    /* 3. Stored source with valid ZIP_STAT_COMP_SIZE: len == -1 (start <= comp_size vs start > comp_size). */
    {FIXTURE_PENDING_STORED_COMP_SIZE, 4, -1, 0, "456789", 6},
    {FIXTURE_PENDING_STORED_COMP_SIZE, 10, -1, 0, "", 0},
    {FIXTURE_PENDING_STORED_COMP_SIZE, 11, -1, ZIP_ER_INCONS, NULL, 0},
    {FIXTURE_ARCHIVE_STORED, 4, -1, 0, "456789", 6},
    {FIXTURE_ARCHIVE_STORED, 10, -1, 0, "", 0},
    {FIXTURE_ARCHIVE_STORED_MISMATCH, 11, -1, ZIP_ER_INCONS, NULL, 0},
    {FIXTURE_ARCHIVE_STORED_MISMATCH, 4, -1, ZIP_ER_INCONS, NULL, 0},

    /* 4. Stored source with valid ZIP_STAT_COMP_SIZE: start + len > comp_size & uint64 wraparound. */
    {FIXTURE_PENDING_STORED_COMP_SIZE, 8, 4, ZIP_ER_INCONS, NULL, 0},
    {FIXTURE_ARCHIVE_STORED_MISMATCH, 8, 4, ZIP_ER_INCONS, NULL, 0},
    {FIXTURE_PENDING_STORED_COMP_SIZE, ZIP_UINT64_MAX, 1, ZIP_ER_INVAL, NULL, 0},
    {FIXTURE_PENDING_STORED_COMP_SIZE, ZIP_UINT64_MAX, 2, ZIP_ER_INVAL, NULL, 0},
    {FIXTURE_ARCHIVE_STORED, ZIP_UINT64_MAX, 2, ZIP_ER_INVAL, NULL, 0},
};

static void test_source_zip_partial(void **state) {
    test_case_t *tc = (test_case_t *)*state;
    stream_ctx_t sctx;
    zip_uint8_t archive_buf[512];
    zip_error_t err;
    zip_t *za;
    zip_source_t *partial;
    char read_buf[16];
    zip_int64_t n;

    za = open_fixture_archive(tc->fixture, &sctx, archive_buf, sizeof(archive_buf));
    zip_error_init(&err);

    partial = zip_source_zip_file_create(za, 0, 0, tc->start, tc->length, NULL, &err);
    if (tc->expected_error != 0) {
        assert_null(partial);
        assert_int_equal(zip_error_code_zip(&err), tc->expected_error);
        zip_error_fini(&err);
        zip_discard(za);
        return;
    }

    assert_non_null(partial);
    assert_int_equal(zip_source_open(partial), 0);
    n = zip_source_read(partial, read_buf, sizeof(read_buf));
    assert_true(n >= 0);
    assert_true((size_t)n == tc->expected_length);
    if (tc->expected_length > 0) {
        assert_memory_equal(read_buf, tc->expected_data, tc->expected_length);
    }
    assert_int_equal(zip_source_read(partial, read_buf, sizeof(read_buf)), 0);
    assert_int_equal(zip_source_close(partial), 0);
    zip_source_free(partial);

    zip_error_fini(&err);
    zip_discard(za);
}

int main(void) {
    return run_test_table("source_zip_partial", test_source_zip_partial, (void *)tests, sizeof(tests[0]), sizeof(tests) / sizeof(tests[0]));
}
