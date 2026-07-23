/*
  exact_read_integrity.c -- verify close reports deferred integrity failures
  Copyright (C) 2026

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

struct nonseekable_context {
    const char *data;
    zip_uint64_t length;
    zip_uint64_t offset;
    zip_error_t error;
};


static zip_int64_t
nonseekable_source(void *userdata, void *data, zip_uint64_t length, zip_source_cmd_t command) {
    struct nonseekable_context *ctx = (struct nonseekable_context *)userdata;

    switch (command) {
    case ZIP_SOURCE_CLOSE:
        return 0;

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&ctx->error, data, length);

    case ZIP_SOURCE_FREE:
        return 0;

    case ZIP_SOURCE_OPEN:
        ctx->offset = 0;
        return 0;

    case ZIP_SOURCE_READ:
        if (length > ctx->length - ctx->offset) {
            length = ctx->length - ctx->offset;
        }
        memcpy(data, ctx->data + ctx->offset, (size_t)length);
        ctx->offset += length;
        return (zip_int64_t)length;

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st;

        if (length < sizeof(*st)) {
            zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
            return -1;
        }
        st = (zip_stat_t *)data;
        zip_stat_init(st);
        st->size = ctx->length;
        st->valid = ZIP_STAT_SIZE;
        return sizeof(*st);
    }

    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_READABLE | ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SUPPORTS_REOPEN);

    default:
        zip_error_set(&ctx->error, ZIP_ER_OPNOTSUPP, 0);
        return -1;
    }
}


static int
check_case(const char *archive, const char *password, const char *name, int expected_close_err) {
    int err;
    int close_err;
    int fail;
    char *buf;
    zip_file_t *zf;
    zip_int64_t n;
    zip_stat_t st;
    zip_t *za;

    fail = 0;
    err = 0;
    buf = NULL;
    za = zip_open(archive, 0, &err);
    if (za == NULL) {
        zip_error_t zip_error;

        zip_error_init_with_code(&zip_error, err);
        fprintf(stderr, "can't open '%s': %s\n", archive, zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return 1;
    }

    if (password != NULL && zip_set_default_password(za, password) < 0) {
        fprintf(stderr, "can't set password for '%s': %s\n", archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    zip_stat_init(&st);
    if (zip_stat(za, name, 0, &st) < 0) {
        fprintf(stderr, "can't stat '%s' in '%s': %s\n", name, archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    if ((zf = zip_fopen(za, name, 0)) == NULL) {
        fprintf(stderr, "can't open '%s' in '%s': %s\n", name, archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    if ((buf = (char *)malloc((size_t)st.size)) == NULL) {
        fprintf(stderr, "malloc failed for '%s'\n", name);
        zip_fclose(zf);
        zip_close(za);
        return 1;
    }

    n = zip_fread(zf, buf, st.size);
    if (n != (zip_int64_t)st.size) {
        if (n < 0) {
            fprintf(stderr, "zip_fread unexpectedly failed for '%s' in '%s': %s\n", name, archive, zip_file_strerror(zf));
        }
        else {
            fprintf(stderr, "zip_fread unexpectedly returned %lld instead of %llu for '%s' in '%s'\n", (long long)n, (unsigned long long)st.size, name, archive);
        }
        fail = 1;
    }

    close_err = zip_fclose(zf);
    if (close_err != expected_close_err) {
        fprintf(stderr, "zip_fclose returned %d instead of %d for '%s' in '%s'\n", close_err, expected_close_err, name, archive);
        fail = 1;
    }

    free(buf);
    zip_close(za);

    return fail;
}


static int
check_nonseekable_reopen(void) {
    static const char input[] = "abc";
    char output[sizeof(input)] = {0};
    struct nonseekable_context ctx;
    zip_error_t error;
    zip_source_t *source;

    ctx.data = input;
    ctx.length = sizeof(input) - 1;
    ctx.offset = 0;
    zip_error_init(&ctx.error);
    zip_error_init(&error);
    source = zip_source_function_create(nonseekable_source, &ctx, &error);
    if (source == NULL) {
        fprintf(stderr, "can't create non-seekable source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&ctx.error);
        zip_error_fini(&error);
        return 1;
    }

    if (zip_source_open(source) < 0 || zip_source_read(source, output, 1) != 1 || zip_source_at_eof(source) != 0 || zip_source_close(source) < 0) {
        fprintf(stderr, "can't prime non-seekable source\n");
        zip_source_free(source);
        zip_error_fini(&ctx.error);
        zip_error_fini(&error);
        return 1;
    }
    if (zip_source_open(source) < 0 || zip_source_read(source, output, sizeof(input) - 1) != sizeof(input) - 1 || memcmp(input, output, sizeof(input) - 1) != 0) {
        fprintf(stderr, "reopened non-seekable source returned wrong data\n");
        zip_source_free(source);
        zip_error_fini(&ctx.error);
        zip_error_fini(&error);
        return 1;
    }

    (void)zip_source_close(source);
    zip_source_free(source);
    zip_error_fini(&ctx.error);
    zip_error_fini(&error);
    return 0;
}


static int
check_reopened_source(void) {
    static const char input[] = "abc";
    char output[sizeof(input)] = {0};
    zip_error_t error;
    zip_source_t *source;

    zip_error_init(&error);
    source = zip_source_buffer_create(input, sizeof(input) - 1, 0, &error);
    if (source == NULL) {
        fprintf(stderr, "can't create buffer source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    if (zip_source_open(source) < 0 || zip_source_seek(source, -1, SEEK_SET) == 0) {
        fprintf(stderr, "can't trigger source error\n");
        zip_source_free(source);
        zip_error_fini(&error);
        return 1;
    }
    (void)zip_source_close(source);

    if (zip_source_open(source) < 0 || zip_source_read(source, output, sizeof(input) - 1) != sizeof(input) - 1 || memcmp(input, output, sizeof(input) - 1) != 0) {
        fprintf(stderr, "can't read reopened source\n");
        zip_source_free(source);
        zip_error_fini(&error);
        return 1;
    }
    if (zip_source_close(source) < 0) {
        fprintf(stderr, "closing reopened source failed: %s\n", zip_error_strerror(zip_source_error(source)));
        zip_source_free(source);
        zip_error_fini(&error);
        return 1;
    }

    zip_source_free(source);
    zip_error_fini(&error);
    return 0;
}


int
main(void) {
    int fail;

    fail = 0;
    fail += check_case("broken.zip", NULL, "storedok", 0);
    fail += check_case("broken.zip", NULL, "storedcrcerror", ZIP_ER_CRC);
    fail += check_case("broken.zip", NULL, "deflatecrcerror", ZIP_ER_CRC);
    fail += check_case("hmac-error.zip", "1234", "test.txt", ZIP_ER_CRC);
    fail += check_nonseekable_reopen();
    fail += check_reopened_source();

    return fail ? 1 : 0;
}
