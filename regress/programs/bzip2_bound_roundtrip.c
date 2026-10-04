/*
 bzip2_bound_roundtrip.c -- roundtrip data that expands under bzip2
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

/* Writes an archive into a growable memory source, compressing an
   incompressible payload with bzip2, so the compressed data is larger
   than the input. The archive is then read back from the same source
   and the payload is compared byte by byte. The roundtrip covers the
   expanding path of the bzip2 compression code, whose sized worst
   case estimate must stay above the expansion bzip2 actually
   produces. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

#define DATA_SIZE (4u * 1024u * 1024u)

static unsigned char payload[DATA_SIZE];

struct memory_buffer {
    unsigned char *data;
    zip_uint64_t length;
    zip_uint64_t capacity;
    zip_uint64_t position;
};

static zip_int64_t buffer_read(struct memory_buffer *buf, void *data, zip_uint64_t len) {
    zip_uint64_t remaining = buf->length - buf->position;

    if (len > remaining) {
        len = remaining;
    }
    memcpy(data, buf->data + buf->position, (size_t)len);
    buf->position += len;
    return (zip_int64_t)len;
}

static int buffer_grow(struct memory_buffer *buf, zip_uint64_t needed) {
    unsigned char *new_data;

    if (needed <= buf->capacity) {
        return 0;
    }
    if (buf->capacity == 0) {
        buf->capacity = 1024;
    }
    while (buf->capacity < needed) {
        if (buf->capacity > ZIP_UINT64_MAX / 2) {
            return -1;
        }
        buf->capacity *= 2;
    }
    if ((new_data = (unsigned char *)realloc(buf->data, (size_t)buf->capacity)) == NULL) {
        return -1;
    }
    buf->data = new_data;
    return 0;
}

static zip_int64_t buffer_write(struct memory_buffer *buf, const void *data, zip_uint64_t len) {
    if (buffer_grow(buf, buf->position + len) < 0) {
        return -1;
    }
    memcpy(buf->data + buf->position, data, (size_t)len);
    buf->position += len;
    if (buf->position > buf->length) {
        buf->length = buf->position;
    }
    return (zip_int64_t)len;
}

static zip_int64_t buffer_seek(struct memory_buffer *buf, void *data, zip_uint64_t len) {
    zip_source_args_seek_t *args;
    zip_int64_t offset;

    if (len != sizeof(*args)) {
        return -1;
    }
    args = (zip_source_args_seek_t *)data;
    offset = args->offset;

    switch (args->whence) {
    case SEEK_SET:
        if (offset < 0) {
            return -1;
        }
        buf->position = (zip_uint64_t)offset;
        return 0;

    case SEEK_CUR:
        if (offset < 0) {
            if ((zip_uint64_t)(-offset) > buf->position) {
                return -1;
            }
            buf->position -= (zip_uint64_t)(-offset);
        }
        else {
            if ((zip_uint64_t)offset > ZIP_UINT64_MAX - buf->position) {
                return -1;
            }
            buf->position += (zip_uint64_t)offset;
        }
        return 0;

    case SEEK_END:
        if (offset < 0) {
            if ((zip_uint64_t)(-offset) > buf->length) {
                return -1;
            }
            buf->position = buf->length - (zip_uint64_t)(-offset);
        }
        else {
            buf->position = buf->length + (zip_uint64_t)offset;
        }
        return 0;

    default:
        return -1;
    }
}

static zip_int64_t memory_source_callback(void *ud, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    struct memory_buffer *buf = (struct memory_buffer *)ud;

    switch (cmd) {
    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_WRITABLE | ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SUPPORTS_REOPEN);

    case ZIP_SOURCE_OPEN:
        buf->position = 0;
        return 0;

    case ZIP_SOURCE_READ:
        return buffer_read(buf, data, len);

    case ZIP_SOURCE_CLOSE:
        return 0;

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st = (zip_stat_t *)data;
        zip_stat_init(st);
        st->size = buf->length;
        st->comp_size = buf->length;
        st->comp_method = ZIP_CM_STORE;
        st->encryption_method = ZIP_EM_NONE;
        st->valid = ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;
        return (zip_int64_t)sizeof(*st);
    }

    case ZIP_SOURCE_ERROR:
        if (len < sizeof(int) * 2) {
            return -1;
        }
        ((int *)data)[0] = ZIP_ER_INTERNAL;
        ((int *)data)[1] = 0;
        return (zip_int64_t)(sizeof(int) * 2);

    case ZIP_SOURCE_FREE:
        free(buf->data);
        free(buf);
        return 0;

    case ZIP_SOURCE_SEEK:
        return buffer_seek(buf, data, len);

    case ZIP_SOURCE_TELL:
        return (zip_int64_t)buf->position;

    case ZIP_SOURCE_BEGIN_WRITE:
        buf->length = 0;
        buf->position = 0;
        return 0;

    case ZIP_SOURCE_COMMIT_WRITE:
        return 0;

    case ZIP_SOURCE_ROLLBACK_WRITE:
        buf->length = 0;
        buf->position = 0;
        return 0;

    case ZIP_SOURCE_WRITE:
        return buffer_write(buf, data, len);

    case ZIP_SOURCE_SEEK_WRITE:
        return buffer_seek(buf, data, len);

    case ZIP_SOURCE_TELL_WRITE:
        return (zip_int64_t)buf->position;

    case ZIP_SOURCE_SUPPORTS_REOPEN:
        return 1;

    default:
        return -1;
    }
}

static zip_source_t *memory_source_new(zip_error_t *error) {
    struct memory_buffer *buf = (struct memory_buffer *)calloc(1, sizeof(*buf));

    if (buf == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }
    return zip_source_function_create(memory_source_callback, buf, error);
}

static void fill_payload(void) {
    /* xorshift64 produces output that bzip2 cannot compress, so the
       stored entry grows instead of shrinking */
    zip_uint64_t state = 88172645463325252ull;
    size_t i;

    for (i = 0; i < DATA_SIZE; i += sizeof(zip_uint64_t)) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        memcpy(payload + i, &state, sizeof(state));
    }
}

int main(void) {
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_file_t *zf;
    char buffer[8192];
    zip_uint64_t total;
    zip_int64_t n;

    fill_payload();

    zip_error_init(&error);
    src = memory_source_new(&error);
    if (src == NULL) {
        fprintf(stderr, "can't create memory source: %s\n", zip_error_strerror(&error));
        return 1;
    }

    za = zip_open_from_source(src, ZIP_CREATE, &error);
    if (za == NULL) {
        fprintf(stderr, "can't open archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        zip_error_fini(&error);
        return 1;
    }

    {
        zip_source_t *entry = zip_source_buffer(za, payload, sizeof(payload), 0);
        if (entry == NULL || zip_file_add(za, "payload.bin", entry, ZIP_FL_ENC_UTF_8) < 0 || zip_set_file_compression(za, 0, ZIP_CM_BZIP2, 1) < 0) {
            fprintf(stderr, "can't populate archive: %s\n", zip_strerror(za));
            zip_discard(za);
            return 1;
        }
    }

    zip_source_keep(src);
    if (zip_close(za) < 0) {
        fprintf(stderr, "can't write archive\n");
        zip_source_free(src);
        return 1;
    }

    za = zip_open_from_source(src, 0, &error);
    if (za == NULL) {
        fprintf(stderr, "can't reopen archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        zip_error_fini(&error);
        return 1;
    }
    zip_error_fini(&error);

    zf = zip_fopen_index(za, 0, 0);
    if (zf == NULL) {
        fprintf(stderr, "can't open file: %s\n", zip_strerror(za));
        zip_discard(za);
        return 1;
    }
    total = 0;
    while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
        if (memcmp(buffer, payload + total, (size_t)n) != 0) {
            fprintf(stderr, "data mismatch at offset %llu\n", (unsigned long long)total);
            zip_fclose(zf);
            zip_discard(za);
            return 1;
        }
        total += (zip_uint64_t)n;
    }
    zip_fclose(zf);
    zip_discard(za);

    if (total != DATA_SIZE) {
        fprintf(stderr, "short read\n");
        return 1;
    }
    return 0;
}
