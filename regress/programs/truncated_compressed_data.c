/* Verify a truncated deflate stream is rejected even when its CRC and size match. */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include "zip.h"

static const unsigned char archive_data[] = {
    0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x59, 0x1a,
    0x4e, 0x5d, 0xc6, 0x35, 0xb9, 0x3b, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x66, 0x69, 0x6c, 0x65, 0x2e, 0x74,
    0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0xe1, 0x02, 0x50, 0x4b, 0x01, 0x02,
    0x14, 0x03, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x59, 0x1a, 0x4e, 0x5d,
    0xc6, 0x35, 0xb9, 0x3b, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x66, 0x69, 0x6c, 0x65, 0x2e, 0x74,
    0x78, 0x74, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x36, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00,
};

int
main(void) {
    zip_error_t error;
    zip_source_t *source;
    zip_t *archive;
    zip_file_t *file;
    zip_int64_t n;
    size_t total;
    int failed;
    char buffer[16];
    char output[16];

    zip_error_init(&error);
    source = zip_source_buffer_create(archive_data, sizeof(archive_data), 0, &error);
    if (source == NULL) {
        fprintf(stderr, "cannot create source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }
    archive = zip_open_from_source(source, ZIP_CHECKCONS, &error);
    if (archive == NULL) {
        fprintf(stderr, "cannot open archive: %s\n", zip_error_strerror(&error));
        zip_source_free(source);
        zip_error_fini(&error);
        return 1;
    }
    zip_error_fini(&error);
    file = zip_fopen_index(archive, 0, 0);
    if (file == NULL) {
        fprintf(stderr, "cannot open entry: %s\n", zip_strerror(archive));
        zip_discard(archive);
        return 1;
    }
    total = 0;
    while ((n = zip_fread(file, buffer, sizeof(buffer))) > 0) {
        if ((size_t)n > sizeof(output) - total) {
            zip_fclose(file);
            zip_discard(archive);
            return 1;
        }
        memcpy(output + total, buffer, (size_t)n);
        total += (size_t)n;
    }
    failed = n != -1 || total != 5 || memcmp(output, "test\n", 5) != 0 ||
             zip_error_code_zip(zip_file_get_error(file)) != ZIP_ER_INCONS ||
             strcmp(zip_file_strerror(file), "Zip archive inconsistent: compressed data truncated") != 0;
    if (failed) {
        fprintf(stderr, "truncated stream not rejected as expected: %s\n", zip_file_strerror(file));
    }
    zip_fclose(file);
    zip_discard(archive);
    return failed;
}
