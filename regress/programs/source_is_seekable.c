#include <stdlib.h>

#include "zip.h"

static zip_int64_t
source_callback(void *ud, void *data, zip_uint64_t length, zip_source_cmd_t command) {
    zip_error_t *error = (zip_error_t *)ud;

    switch (command) {
    case ZIP_SOURCE_CLOSE:
    case ZIP_SOURCE_FREE:
    case ZIP_SOURCE_OPEN:
        return 0;

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(error, data, length);

    case ZIP_SOURCE_READ:
        return 0;

    case ZIP_SOURCE_SUPPORTS:
        return zip_source_make_command_bitmap(ZIP_SOURCE_OPEN, ZIP_SOURCE_READ, ZIP_SOURCE_CLOSE, ZIP_SOURCE_ERROR, ZIP_SOURCE_FREE, -1);

    default:
        zip_error_set(error, ZIP_ER_OPNOTSUPP, 0);
        return -1;
    }
}


int
main(void) {
    zip_error_t error;
    zip_source_t *seekable;
    zip_source_t *nonseekable;

    zip_error_init(&error);

    seekable = zip_source_buffer_create("A", 1, 0, &error);
    if (seekable == NULL) {
        zip_error_fini(&error);
        return 1;
    }
    if (zip_source_is_seekable(seekable) != 1) {
        zip_source_free(seekable);
        zip_error_fini(&error);
        return 1;
    }
    zip_source_free(seekable);

    nonseekable = zip_source_function_create(source_callback, &error, &error);
    if (nonseekable == NULL) {
        zip_error_fini(&error);
        return 1;
    }
    if (zip_source_is_seekable(nonseekable) != 0) {
        zip_source_free(nonseekable);
        zip_error_fini(&error);
        return 1;
    }
    zip_source_free(nonseekable);

    zip_error_fini(&error);
    return 0;
}
