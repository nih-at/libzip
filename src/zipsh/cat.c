#include <errno.h>
#include <string.h>

#include "zipsh.h"

bool cat_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_error_t error;
    zip_source_t *src;
    zip_int64_t n;
    char buf[8192];
    zip_uint64_t start = invocation->arguments[1].u64;
    zip_int64_t len = invocation->arguments[2].i64;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    zip_error_init(&error);
    if ((src = zip_source_zip_file_create(archive, file_ref->index, 0, start, len, NULL, &error)) == NULL) {
        *error_message = error_message_new_format("can't open file %s: %s", file_ref->error_info, zip_error_strerror(&error));
        zip_error_fini(&error);
        file_reference_free(file_ref);
        return false;
    }
    zip_error_fini(&error);

    if (zip_source_open(src) < 0) {
        *error_message = error_message_new_format("can't open file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        file_reference_free(file_ref);
        return false;
    }
    while ((n = zip_source_read(src, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, (size_t)n, 1, stdout) != 1) {
            if (error_message) {
                *error_message = error_message_new_format("can't write file contents: %s", strerror(errno));
            }
            zip_source_free(src);
            file_reference_free(file_ref);
            return false;
        }
    }
    if (n == -1) {
        *error_message = error_message_new_format("can't read file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        file_reference_free(file_ref);
        return false;
    }
    if (zip_source_close(src) < 0) {
        *error_message = error_message_new_format("can't close file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        file_reference_free(file_ref);
        return false;
    }
    zip_source_free(src);
    file_reference_free(file_ref);

    return true;
}

/* clang-format off */
argument_definition_t cat_arguments[] = {
    {"file", "file to display", ARGUMENT_TYPE_STRING, true, {.string = NULL}},
    {"start", "starting offset in the file", ARGUMENT_TYPE_UINT64, false, {.u64 = 0}},
    {"length", "number of bytes to read from the file", ARGUMENT_TYPE_INT64, false, {.i64 = -1}},
};

option_definition_t cat_options[] = {
    {'i', "index", NULL, "file argument is index instead of name", ARGUMENT_TYPE_BOOL},
};
/* clang-format on */

COMMAND(cat, "show contents of file in archive");
