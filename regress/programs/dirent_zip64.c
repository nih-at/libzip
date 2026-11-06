/*
 dirent_zip64.c -- verify Zip64 size and offset boundary serialization
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

#include <stdio.h>
#include <string.h>

#include "cmocka_utilities.h"
#include "zipint.h"

#define BOUNDARY ZIP_UINT32_MAX
#define ABOVE_BOUNDARY ((zip_uint64_t)ZIP_UINT32_MAX + 1)

struct test_data {
    zip_flags_t flags;
    zip_uint64_t uncomp_size;
    zip_uint64_t comp_size;
    zip_uint64_t offset;
    zip_uint32_t header_uncomp_size;
    zip_uint32_t header_comp_size;
    zip_uint16_t version_needed;
    zip_uint16_t zip64_length;
    zip_uint64_t zip64_values[3];
};

typedef struct test_data test_data_t;

/* The sizes and offsets describe metadata only; no entry data is allocated. */
/* clang-format off */
static const test_data_t test_data[] = {
    {ZIP_FL_CENTRAL, 7, 5, 11, 7, 5, 10, 0, {0}},
    {ZIP_FL_CENTRAL, BOUNDARY, 5, 11, BOUNDARY, 5, 10, 0, {0}},
    {ZIP_FL_CENTRAL, 7, BOUNDARY, 11, 7, BOUNDARY, 10, 0, {0}},
    {ZIP_FL_CENTRAL, BOUNDARY, BOUNDARY, BOUNDARY, BOUNDARY, BOUNDARY, 10, 0, {0}},
    {ZIP_FL_CENTRAL, ABOVE_BOUNDARY, 5, 11, BOUNDARY, 5, 45, 8, {ABOVE_BOUNDARY}},
    {ZIP_FL_CENTRAL, 7, ABOVE_BOUNDARY, 11, 7, BOUNDARY, 45, 8, {ABOVE_BOUNDARY}},
    {ZIP_FL_CENTRAL, BOUNDARY, ABOVE_BOUNDARY, 11, BOUNDARY, BOUNDARY, 45, 16, {BOUNDARY, ABOVE_BOUNDARY}},
    {ZIP_FL_CENTRAL, ABOVE_BOUNDARY, BOUNDARY, 11, BOUNDARY, BOUNDARY, 45, 16, {ABOVE_BOUNDARY, BOUNDARY}},
    {ZIP_FL_CENTRAL, 7, 5, BOUNDARY, 7, 5, 10, 0, {0}},
    {ZIP_FL_CENTRAL, 7, 5, ABOVE_BOUNDARY, 7, 5, 45, 8, {ABOVE_BOUNDARY}},
    {ZIP_FL_CENTRAL, ABOVE_BOUNDARY, 5, BOUNDARY, BOUNDARY, 5, 45, 16, {ABOVE_BOUNDARY, BOUNDARY}},
    {ZIP_FL_CENTRAL, BOUNDARY, BOUNDARY, ABOVE_BOUNDARY, BOUNDARY, BOUNDARY, 45, 24, {BOUNDARY, BOUNDARY, ABOVE_BOUNDARY}},
    {ZIP_FL_LOCAL, 7, 5, 0, 7, 5, 10, 0, {0}},
    {ZIP_FL_LOCAL, BOUNDARY, 5, 0, BOUNDARY, 5, 10, 0, {0}},
    {ZIP_FL_LOCAL, 7, BOUNDARY, 0, 7, BOUNDARY, 10, 0, {0}},
    {ZIP_FL_LOCAL, BOUNDARY, BOUNDARY, 0, BOUNDARY, BOUNDARY, 10, 0, {0}},
    {ZIP_FL_LOCAL, ABOVE_BOUNDARY, 5, 0, BOUNDARY, BOUNDARY, 45, 16, {ABOVE_BOUNDARY, 5}},
    {ZIP_FL_LOCAL, 7, ABOVE_BOUNDARY, 0, BOUNDARY, BOUNDARY, 45, 16, {7, ABOVE_BOUNDARY}},
    {ZIP_FL_LOCAL, BOUNDARY, ABOVE_BOUNDARY, 0, BOUNDARY, BOUNDARY, 45, 16, {BOUNDARY, ABOVE_BOUNDARY}},
    {ZIP_FL_LOCAL, ABOVE_BOUNDARY, BOUNDARY, 0, BOUNDARY, BOUNDARY, 45, 16, {ABOVE_BOUNDARY, BOUNDARY}},
    {ZIP_FL_LOCAL, 7, 5, ABOVE_BOUNDARY, 7, 5, 10, 0, {0}},
    {ZIP_FL_LOCAL | ZIP_FL_FORCE_ZIP64, 7, 5, 0, 7, 5, 45, 16, {7, 5}},
    {ZIP_FL_LOCAL | ZIP_FL_FORCE_ZIP64, BOUNDARY, 5, 0, BOUNDARY, BOUNDARY, 45, 16, {BOUNDARY, 5}},
    {ZIP_FL_LOCAL | ZIP_FL_FORCE_ZIP64, 7, BOUNDARY, 0, BOUNDARY, BOUNDARY, 45, 16, {7, BOUNDARY}}
};
/* clang-format on */

static void check_dirent(void **state) {
    const test_data_t *test = (const test_data_t *)*state;
    bool local = (test->flags & ZIP_FL_LOCAL) != 0;
    zip_t archive;
    zip_dirent_t dirent, read_dirent;
    zip_buffer_t *buffer;
    zip_uint8_t data[128];
    zip_int64_t length;
    zip_uint64_t header_length = local ? LENTRYSIZE : CDENTRYSIZE;
    zip_uint16_t extra_length = test->zip64_length == 0 ? 0 : test->zip64_length + 4;
    size_t i;

    memset(&archive, 0, sizeof(archive));
    zip_error_init(&archive.error);
    archive.src = zip_source_buffer_create(NULL, 0, 0, &archive.error);
    assert_non_null(archive.src);
    assert_int_equal(zip_source_begin_write(archive.src), 0);

    _zip_dirent_init(&dirent);
    if (test->flags & ZIP_FL_FORCE_ZIP64) {
        dirent.version_needed = 45;
    }
    dirent.filename = _zip_string_new((const zip_uint8_t *)"a", 1, ZIP_FL_ENC_GUESS, &archive.error);
    assert_non_null(dirent.filename);
    dirent.comp_method = ZIP_CM_DEFLATE;
    dirent.uncomp_size = test->uncomp_size;
    dirent.comp_size = test->comp_size;
    dirent.offset = test->offset;

    assert_int_equal(_zip_dirent_write(&archive, &dirent, test->flags), test->zip64_length != 0);
    assert_int_equal(zip_source_commit_write(archive.src), 0);
    assert_int_equal(zip_source_open(archive.src), 0);
    length = zip_source_read(archive.src, data, sizeof(data));
    assert_int_equal(length, header_length + 1 + extra_length);
    assert_int_equal(zip_source_close(archive.src), 0);

    buffer = _zip_buffer_new(data, (zip_uint64_t)length);
    assert_non_null(buffer);
    assert_int_equal(_zip_buffer_set_offset(buffer, local ? 4 : 6), 0);
    assert_int_equal(_zip_buffer_get_16(buffer), test->version_needed);
    assert_int_equal(_zip_buffer_set_offset(buffer, local ? 18 : 20), 0);
    assert_int_equal(_zip_buffer_get_32(buffer), test->header_comp_size);
    assert_int_equal(_zip_buffer_get_32(buffer), test->header_uncomp_size);
    assert_int_equal(_zip_buffer_get_16(buffer), 1);
    assert_int_equal(_zip_buffer_get_16(buffer), extra_length);
    if (!local) {
        assert_int_equal(_zip_buffer_set_offset(buffer, 42), 0);
        assert_int_equal(_zip_buffer_get_32(buffer), test->offset > BOUNDARY ? BOUNDARY : test->offset);
    }
    assert_int_equal(_zip_buffer_set_offset(buffer, header_length + 1), 0);
    if (extra_length != 0) {
        assert_int_equal(_zip_buffer_get_16(buffer), ZIP_EF_ZIP64);
        assert_int_equal(_zip_buffer_get_16(buffer), test->zip64_length);
        for (i = 0; i < test->zip64_length / 8; i++) {
            assert_int_equal(_zip_buffer_get_64(buffer), test->zip64_values[i]);
        }
    }
    assert_true(_zip_buffer_eof(buffer));

    assert_int_equal(_zip_buffer_set_offset(buffer, 0), 0);
    assert_int_equal(_zip_dirent_read(&read_dirent, NULL, buffer, local, extra_length != 0, test->comp_size, false, &archive.error), length);
    assert_int_equal(read_dirent.uncomp_size, test->uncomp_size);
    assert_int_equal(read_dirent.comp_size, test->comp_size);
    assert_int_equal(read_dirent.offset, local ? 0 : test->offset);
    _zip_dirent_finalize(&read_dirent);
    _zip_buffer_free(buffer);
    _zip_dirent_finalize(&dirent);
    zip_source_free(archive.src);
    zip_error_fini(&archive.error);
}

struct clone_test_data {
    zip_uint64_t size;
    bool wide_descriptor;
    bool keep_descriptor;
};

typedef struct clone_test_data clone_test_data_t;

static const clone_test_data_t clone_test_data[] = {{7, false, true}, {BOUNDARY, false, false}, {BOUNDARY, true, false}};

static zip_uint64_t make_descriptor_archive(zip_uint8_t *data, zip_uint64_t capacity, const clone_test_data_t *test, zip_uint64_t *prefix_length) {
    zip_buffer_t *buffer = _zip_buffer_new(data, capacity);
    zip_uint64_t central_size, central_end, length;

    assert_non_null(buffer);
    _zip_buffer_put(buffer, LOCAL_MAGIC, 4);
    _zip_buffer_put_16(buffer, test->wide_descriptor ? 45 : 20);
    _zip_buffer_put_16(buffer, ZIP_GPBF_DATA_DESCRIPTOR);
    _zip_buffer_put_16(buffer, ZIP_CM_DEFLATE);
    _zip_buffer_put_16(buffer, 0);
    _zip_buffer_put_16(buffer, 0x21);
    _zip_buffer_put_32(buffer, 0);
    _zip_buffer_put_32(buffer, test->wide_descriptor ? BOUNDARY : 2);
    _zip_buffer_put_32(buffer, (zip_uint32_t)test->size);
    _zip_buffer_put_16(buffer, 1);
    _zip_buffer_put_16(buffer, test->wide_descriptor ? 20 : 0);
    _zip_buffer_put(buffer, "a", 1);
    if (test->wide_descriptor) {
        _zip_buffer_put_16(buffer, ZIP_EF_ZIP64);
        _zip_buffer_put_16(buffer, 16);
        _zip_buffer_put_64(buffer, test->size);
        _zip_buffer_put_64(buffer, 2);
    }
    _zip_buffer_put(buffer, "\003\000", 2);
    _zip_buffer_put(buffer, DATADES_MAGIC, 4);
    _zip_buffer_put_32(buffer, 0);
    if (test->wide_descriptor) {
        _zip_buffer_put_64(buffer, 2);
        _zip_buffer_put_64(buffer, test->size);
    }
    else {
        _zip_buffer_put_32(buffer, 2);
        _zip_buffer_put_32(buffer, (zip_uint32_t)test->size);
    }
    *prefix_length = _zip_buffer_offset(buffer);

    _zip_buffer_put(buffer, CENTRAL_MAGIC, 4);
    _zip_buffer_put_16(buffer, 0x031e);
    _zip_buffer_put_16(buffer, test->wide_descriptor ? 45 : 20);
    _zip_buffer_put_16(buffer, ZIP_GPBF_DATA_DESCRIPTOR);
    _zip_buffer_put_16(buffer, ZIP_CM_DEFLATE);
    _zip_buffer_put_16(buffer, 0);
    _zip_buffer_put_16(buffer, 0x21);
    _zip_buffer_put_32(buffer, 0);
    _zip_buffer_put_32(buffer, 2);
    _zip_buffer_put_32(buffer, (zip_uint32_t)test->size);
    _zip_buffer_put_16(buffer, 1);
    _zip_buffer_put_16(buffer, test->wide_descriptor ? 12 : 0);
    _zip_buffer_put_16(buffer, 0);
    _zip_buffer_put_16(buffer, 0);
    _zip_buffer_put_16(buffer, 0);
    _zip_buffer_put_32(buffer, 0);
    _zip_buffer_put_32(buffer, 0);
    _zip_buffer_put(buffer, "a", 1);
    if (test->wide_descriptor) {
        _zip_buffer_put_16(buffer, ZIP_EF_ZIP64);
        _zip_buffer_put_16(buffer, 8);
        _zip_buffer_put_64(buffer, test->size);
    }
    central_end = _zip_buffer_offset(buffer);
    central_size = central_end - *prefix_length;

    if (test->wide_descriptor) {
        _zip_buffer_put(buffer, EOCD64_MAGIC, 4);
        _zip_buffer_put_64(buffer, EOCD64LEN - 12);
        _zip_buffer_put_16(buffer, 45);
        _zip_buffer_put_16(buffer, 45);
        _zip_buffer_put_32(buffer, 0);
        _zip_buffer_put_32(buffer, 0);
        _zip_buffer_put_64(buffer, 1);
        _zip_buffer_put_64(buffer, 1);
        _zip_buffer_put_64(buffer, central_size);
        _zip_buffer_put_64(buffer, *prefix_length);
        _zip_buffer_put(buffer, EOCD64LOC_MAGIC, 4);
        _zip_buffer_put_32(buffer, 0);
        _zip_buffer_put_64(buffer, central_end);
        _zip_buffer_put_32(buffer, 1);
    }
    _zip_buffer_put(buffer, EOCD_MAGIC, 4);
    _zip_buffer_put_32(buffer, 0);
    _zip_buffer_put_16(buffer, 1);
    _zip_buffer_put_16(buffer, 1);
    _zip_buffer_put_32(buffer, (zip_uint32_t)central_size);
    _zip_buffer_put_32(buffer, (zip_uint32_t)*prefix_length);
    _zip_buffer_put_16(buffer, 0);
    assert_true(_zip_buffer_ok(buffer));
    length = _zip_buffer_offset(buffer);
    _zip_buffer_free(buffer);
    return length;
}

static void check_descriptor_clone(void **state) {
    const clone_test_data_t *test = (const clone_test_data_t *)*state;
    zip_uint8_t input[256], output[256];
    zip_uint64_t input_length, prefix_length;
    zip_int64_t output_length;
    zip_buffer_fragment_t fragments[2];
    zip_source_t *source;
    zip_t *archive;
    zip_buffer_t *buffer;
    zip_error_t error;
    zip_uint16_t output_flags, extra_length;

    input_length = make_descriptor_archive(input, sizeof(input), test, &prefix_length);
    fragments[0].data = input;
    fragments[0].length = prefix_length;
    fragments[1].data = input + prefix_length;
    fragments[1].length = input_length - prefix_length;
    zip_error_init(&error);
    source = zip_source_buffer_fragment_create(fragments, 2, 0, &error);
    assert_non_null(source);
    assert_true(zip_source_supports(source) & ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_BEGIN_WRITE_CLONING));
    zip_source_keep(source);
    archive = zip_open_from_source(source, ZIP_CHECKCONS, &error);
    assert_non_null(archive);
    assert_int_equal(zip_set_archive_comment(archive, "hi", 2), 0);
    assert_int_equal(zip_close(archive), 0);

    assert_int_equal(zip_source_open(source), 0);
    output_length = zip_source_read(source, output, sizeof(output));
    assert_int_equal(zip_source_close(source), 0);
    assert_true(output_length > 0 && output_length <= (zip_int64_t)sizeof(output));
    buffer = _zip_buffer_new(output, (zip_uint64_t)output_length);
    assert_non_null(buffer);
    assert_int_equal(_zip_buffer_set_offset(buffer, 6), 0);
    output_flags = _zip_buffer_get_16(buffer);
    assert_int_equal(_zip_buffer_set_offset(buffer, 28), 0);
    extra_length = _zip_buffer_get_16(buffer);
    _zip_buffer_free(buffer);

    zip_source_keep(source);
    archive = zip_open_from_source(source, ZIP_CHECKCONS, &error);
    print_message("descriptor: size=%" PRIu64 " wide=%d prefix=%" PRIu64 " output=%" PRId64 " dd=%d reopen=%s\n", test->size, test->wide_descriptor, prefix_length, output_length, (output_flags & ZIP_GPBF_DATA_DESCRIPTOR) != 0, archive == NULL ? zip_error_strerror(&error) : "ok");
    assert_non_null(archive);
    zip_discard(archive);
    assert_int_equal(output_length, test->keep_descriptor ? 120 : 104);
    assert_int_equal(output_flags & ZIP_GPBF_DATA_DESCRIPTOR, test->keep_descriptor ? ZIP_GPBF_DATA_DESCRIPTOR : 0);
    assert_int_equal(extra_length, 0);
    if (test->keep_descriptor) {
        assert_memory_equal(input, output, (size_t)prefix_length);
    }
    zip_source_free(source);
    zip_error_fini(&error);
}

int main(void) {
    int result;

    result = run_test_table("dirent_zip64", check_dirent, (void *)test_data, sizeof(test_data[0]), sizeof(test_data) / sizeof(test_data[0]));
    result += run_test_table("descriptor_clone", check_descriptor_clone, (void *)clone_test_data, sizeof(clone_test_data[0]), sizeof(clone_test_data) / sizeof(clone_test_data[0]));
    return result;
}
