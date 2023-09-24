#include <stdint.h>
#include <zip.h>

/**
   This fuzzer target simulates the process of handling encrypted
   files within a ZIP archive.
**/

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_source_t *src;
    zip_t *za;
    zip_error_t error;
    char buf[32768];

    zip_error_init(&error);
    if ((src = zip_source_buffer_create(data, size, 0, &error)) == NULL) {
        zip_error_fini(&error);
        return 0;
    }

    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
        zip_source_free(src);
        zip_error_fini(&error);
        return 0;
    }
    zip_error_fini(&error);

    int file_index = zip_name_locate(za, "file", 0);

    if (file_index < 0) {
        fprintf(stderr, "Failed to locate encrypted file in ZIP archive\n");
        zip_close(za);
        return 1;
    }

    zip_file_t *file = zip_fopen_index_encrypted(za, file_index, 0, "secretpassword");

    if (!file) {
        fprintf(stderr, "Failed to open encrypted file for reading\n");
        zip_close(za);
        return 1;
    }

    while (zip_fread(file, buf, sizeof(buf)) > 0) {
        ;
    }

    if (zip_fclose(file) < 0) {
        fprintf(stderr, "Failed to close encrypted file\n");
        zip_close(za);
        return 1;
    }
    if (zip_close(za) < 0) {
        fprintf(stderr, "Failed to close archive\n");
        return 1;
    }

    return 0;
}
