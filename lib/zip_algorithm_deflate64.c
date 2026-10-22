/*
  zip_algorithm_deflate64.c -- deflate64 decompression routines
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

#include "zipint.h"

#include <inflatelib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct ctx {
    zip_error_t *error;
    bool end_of_input;
    inflatelib_stream stream;
    zip_uint8_t no_input;
};


static zip_uint64_t maximum_compressed_size(zip_uint64_t uncompressed_size) {
    zip_uint64_t compressed_size = uncompressed_size + (uncompressed_size + 65534) / 65535 * 5 + 6;

    if (compressed_size < uncompressed_size) {
        return ZIP_UINT64_MAX;
    }
    return compressed_size;
}


static void *decompress_allocate(zip_uint16_t method, zip_uint32_t compression_flags, zip_error_t *error) {
    struct ctx *ctx;

    (void)method;
    (void)compression_flags;

    if ((ctx = (struct ctx *)malloc(sizeof(*ctx))) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    ctx->error = error;
    ctx->end_of_input = false;
    memset(&ctx->stream, 0, sizeof(ctx->stream));

    return ctx;
}


static void deallocate(void *ud) {
    struct ctx *ctx = (struct ctx *)ud;

    free(ctx);
}


static zip_uint16_t general_purpose_bit_flags(void *ud) {
    (void)ud;
    return 0;
}


static int map_error(int ret) {
    switch (ret) {
    case INFLATELIB_ERROR_OOM:
        return ZIP_ER_MEMORY;

    case INFLATELIB_ERROR_DATA:
        return ZIP_ER_COMPRESSED_DATA;

    default:
        return ZIP_ER_INTERNAL;
    }
}


static bool start(void *ud, zip_stat_t *st, zip_file_attributes_t *attributes) {
    struct ctx *ctx = (struct ctx *)ud;
    int ret;

    (void)st;
    (void)attributes;

    ctx->end_of_input = false;

    memset(&ctx->stream, 0, sizeof(ctx->stream));
    if ((ret = inflatelib_init(&ctx->stream)) != INFLATELIB_OK) {
        zip_error_set(ctx->error, map_error(ret), 0);
        return false;
    }

    ctx->stream.next_in = &ctx->no_input;
    ctx->stream.avail_in = 0;

    return true;
}


static bool end(void *ud) {
    struct ctx *ctx = (struct ctx *)ud;

    inflatelib_destroy(&ctx->stream);

    return true;
}


static bool input(void *ud, zip_uint8_t *data, zip_uint64_t length) {
    struct ctx *ctx = (struct ctx *)ud;

    if (length > SIZE_MAX || ctx->stream.avail_in > 0) {
        zip_error_set(ctx->error, ZIP_ER_INVAL, 0);
        return false;
    }

    ctx->stream.next_in = data;
    ctx->stream.avail_in = (size_t)length;

    return true;
}


static bool end_of_input(void *ud) {
    struct ctx *ctx = (struct ctx *)ud;

    ctx->end_of_input = true;
    return ctx->stream.avail_in != 0;
}


static zip_compression_status_t process(void *ud, zip_uint8_t *data, zip_uint64_t *length) {
    struct ctx *ctx = (struct ctx *)ud;
    size_t avail_out;
    int ret;

    avail_out = (size_t)ZIP_MIN(SIZE_MAX, *length);
    ctx->stream.next_out = data;
    ctx->stream.avail_out = avail_out;

    ret = inflatelib_inflate64(&ctx->stream);

    *length = avail_out - ctx->stream.avail_out;

    switch (ret) {
    case INFLATELIB_EOF:
        return ZIP_COMPRESSION_END;

    case INFLATELIB_OK:
        if (ctx->stream.avail_out > 0 && ctx->stream.avail_in == 0) {
            if (ctx->end_of_input) {
                zip_error_set(ctx->error, ZIP_ER_COMPRESSED_DATA, 0);
                return ZIP_COMPRESSION_ERROR;
            }
            return ZIP_COMPRESSION_NEED_DATA;
        }
        return ZIP_COMPRESSION_OK;

    default:
        zip_error_set(ctx->error, map_error(ret), 0);
        return ZIP_COMPRESSION_ERROR;
    }
}

/* clang-format off */

zip_compression_algorithm_t zip_algorithm_deflate64_decompress = {
    maximum_compressed_size,
    decompress_allocate,
    deallocate,
    general_purpose_bit_flags,
    21,
    start,
    end,
    input,
    end_of_input,
    process
};

/* clang-format on */
