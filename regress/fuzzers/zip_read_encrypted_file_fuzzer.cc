#include <fstream>
#include <iostream>
#include <string>
#include <zip.h>

#ifdef __cplusplus
extern "C" {
#if 0
    /* fix autoindent */
    }
#endif
#endif

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
    zip_int64_t i, n;
    zip_file_t *f;

    if ((src = zip_source_buffer_create(data, size, 0, &error)) == NULL) {
        zip_error_fini(&error);
        return 0;
    }

    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
        zip_source_free(src);
        zip_error_fini(&error);
        return 0;
    }

    int file_index = zip_name_locate(za, "file", 0);

    if (file_index < 0) {
        std::cerr << "Failed to locate encrypted file in ZIP archive" << std::endl;
        zip_close(za);
        return 1;
    }

    zip_file_t *file = zip_fopen_index_encrypted(za, file_index, 0, "secretpassword");

    if (!file) {
        std::cerr << "Failed to open encrypted file for reading" << std::endl;
        zip_close(za);
        return 1;
    }

    /* TODO: read file */

    if (zip_fclose(file) < 0) {
        std::cerr << "Failed to close encrypted file" << std::endl;
        zip_close(za);
        return 1;
    }
    if (zip_close(za) < 0) {
        std::cerr << "Failed to close archive" << std::endl;
        return 1;
    }

    return 0;
}
#ifdef __cplusplus
}
#endif
