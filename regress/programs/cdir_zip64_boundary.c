/*
  cdir_zip64_boundary.c -- write the Zip64 record when the entry count equals 0xffff
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

/* An archive with exactly 65535 entries has the escape value 0xffff
   in the entry count field of the end of central directory record,
   which directs readers to the Zip64 end of central directory record.
   Writing that many entries must therefore also write the Zip64
   records. The archive is built in a growable memory source built on
   zip_source_function callbacks. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

#define ENTRY_COUNT 65535

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

static const unsigned char EOCD_MAGIC[4] = {0x50, 0x4b, 0x05, 0x06};
static const unsigned char EOCD64LOC_MAGIC[4] = {0x50, 0x4b, 0x06, 0x07};

int main(void) {
    struct memory_buffer *buf = (struct memory_buffer *)calloc(1, sizeof(*buf));
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_uint64_t i;
    zip_int64_t eocd_offset;

    if (buf == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    zip_error_init(&error);
    if ((src = zip_source_function_create(memory_source_callback, buf, &error)) == NULL) {
        fprintf(stderr, "can't create memory source: %s\n", zip_error_strerror(&error));
        return 1;
    }
    if ((za = zip_open_from_source(src, ZIP_CREATE, &error)) == NULL) {
        fprintf(stderr, "can't open archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        return 1;
    }

    for (i = 0; i < ENTRY_COUNT; i++) {
        char name[16];
        zip_source_t *entry;

        snprintf(name, sizeof(name), "f%llu", (unsigned long long)i);
        if ((entry = zip_source_buffer(za, "", 0, 0)) == NULL || zip_file_add(za, name, entry, ZIP_FL_ENC_UTF_8) < 0) {
            fprintf(stderr, "can't add entry %llu: %s\n", (unsigned long long)i, zip_strerror(za));
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

    eocd_offset = (zip_int64_t)buf->length - 22;
    while (eocd_offset >= 0 && memcmp(buf->data + eocd_offset, EOCD_MAGIC, 4) != 0) {
        eocd_offset--;
    }
    if (eocd_offset < 0) {
        fprintf(stderr, "can't find end of central directory record\n");
        zip_source_free(src);
        return 1;
    }

    if (buf->data[eocd_offset + 10] != 0xff || buf->data[eocd_offset + 11] != 0xff) {
        fprintf(stderr, "entry count field is not the escape value\n");
        zip_source_free(src);
        return 1;
    }

    if (eocd_offset < 20 || memcmp(buf->data + eocd_offset - 20, EOCD64LOC_MAGIC, 4) != 0) {
        fprintf(stderr, "entry count 0xffff is written without the Zip64 records\n");
        zip_source_free(src);
        return 1;
    }

    if ((za = zip_open_from_source(src, ZIP_RDONLY, &error)) == NULL) {
        fprintf(stderr, "can't reopen archive: %s\n", zip_error_strerror(&error));
        zip_source_free(src);
        return 1;
    }

    if (zip_get_num_entries(za, 0) != ENTRY_COUNT) {
        fprintf(stderr, "reopened archive has %lld entries\n", (long long)zip_get_num_entries(za, 0));
        zip_unchange_all(za);
        zip_discard(za);
        return 1;
    }

    zip_unchange_all(za);
    zip_discard(za);
    zip_error_fini(&error);
    return 0;
}
