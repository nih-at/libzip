/*
 * zip_write_alt_compress_fuzzer.c
 *
 * Fuzz harness for libzip alternative compression codec write paths:
 * BZIP2 (method 12), ZSTD (method 93), and XZ/LZMA (method 95/14).
 *
 * The existing zip_write_roundtrip_fuzzer covers only STORE and DEFLATE.
 * This harness exercises the remaining codec paths in a full write->read
 * roundtrip, covering lib/zip_algorithm_bzip2.c, zip_algorithm_zstd.c,
 * and zip_algorithm_xz.c.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipint.h"

static uint8_t consume_u8(const uint8_t **data, size_t *size) {
    if (*size == 0) return 0;
    uint8_t v = **data; (*data)++; (*size)--; return v;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 4) return 0;

    static const zip_int32_t methods[] = {
        ZIP_CM_BZIP2, ZIP_CM_ZSTD, ZIP_CM_XZ, ZIP_CM_LZMA, ZIP_CM_STORE,
    };
    static const size_t num_methods = sizeof(methods) / sizeof(methods[0]);

    zip_int32_t  method     = methods[consume_u8(&data, &size) % num_methods];
    zip_uint32_t comp_level = consume_u8(&data, &size) % 10;

    size_t content_len = size / 2;
    size_t comment_len = size - content_len;
    const uint8_t *content = data;
    const uint8_t *comment = data + content_len;

    /* Write phase */
    zip_source_t *src = zip_source_buffer_create(NULL, 0, 1, NULL);
    if (!src) return 0;

    zip_t *za = zip_open_from_source(src, ZIP_TRUNCATE, NULL);
    if (!za) { zip_source_free(src); return 0; }

    zip_source_t *file_src = zip_source_buffer(za, content, content_len, 0);
    if (!file_src) { zip_discard(za); return 0; }

    zip_int64_t idx = zip_file_add(za, "fuzz.bin", file_src, ZIP_FL_ENC_UTF_8);
    if (idx < 0) { zip_discard(za); return 0; }

    zip_set_file_compression(za, (zip_uint64_t)idx, method, comp_level);

    if (comment_len > 0 && comment_len <= 65535)
        zip_set_archive_comment(za, (const char *)comment, (zip_uint16_t)comment_len);

    if (zip_close(za) != 0) return 0;

    /* Read-back phase */
    zip_source_keep(src);
    zip_source_seek(src, 0, SEEK_SET);

    zip_t *za_r = zip_open_from_source(src, ZIP_RDONLY, NULL);
    if (za_r) {
        zip_file_t *zf = zip_fopen_index(za_r, 0, 0);
        if (zf) {
            uint8_t buf[4096];
            while (zip_fread(zf, buf, sizeof(buf)) > 0) {}
            zip_fclose(zf);
        }
        zip_discard(za_r);
    }
    zip_source_free(src);
    return 0;
}
