#include <errno.h>
#include <string.h>

#include "shell.h"
#include "zipsh.h"

#define ARGUMENT_START 1
#define ARGUMENT_LENGTH 2

/* clang-format off */
argument_definition_t cat_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"start", "starting offset in the file", argument_type_uint64, ARGUMENT_OPTIONAL, {.u64 = 0}},
    {"length", "number of bytes to read from the file", argument_type_int64, ARGUMENT_OPTIONAL, {.i64 = -1}},
};

option_definition_t cat_options[] = {
    FILE_REFERENCE_OPTIONS
};

struct cat_context {
    zip_uint64_t start;
    zip_int64_t length;
};

typedef struct cat_context cat_context_t;
/* clang-format on */

bool cat_setup(invocation_t *invocation, void *context, file_matches_t *matches, error_message_t **error_message) {
    cat_context_t *cat_context = (cat_context_t *)context;
    cat_context->start = invocation->arguments[ARGUMENT_START].u64;
    cat_context->length = invocation->arguments[ARGUMENT_LENGTH].i64;

    return true;
}

bool cat_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    cat_context_t *context = (cat_context_t *)context_v;
    zip_error_t error;
    zip_source_t *src;
    zip_int64_t n;
    char buf[8192];

    zip_error_init(&error);

    if ((src = zip_source_zip_file_create(archive, file_ref->index, matches->flags, context->start, context->length, NULL, &error)) == NULL) {
        *error_message = error_message_new_format("can't open file %s: %s", file_ref->error_info, zip_error_strerror(&error));
        zip_error_fini(&error);
        return false;
    }
    zip_error_fini(&error);

    if (zip_source_open(src) < 0) {
        *error_message = error_message_new_format("can't open file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        return false;
    }
    while ((n = zip_source_read(src, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, (size_t)n, 1, stdout) != 1) {
            if (error_message) {
                *error_message = error_message_new_format("can't write file contents: %s", strerror(errno));
            }
            zip_source_free(src);
            return false;
        }
    }
    if (n == -1) {
        *error_message = error_message_new_format("can't read file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        return false;
    }
    if (zip_source_close(src) < 0) {
        *error_message = error_message_new_format("can't close file %s: %s", file_ref->error_info, zip_error_strerror(zip_source_error(src)));
        zip_source_free(src);
        return false;
    }
    zip_source_free(src);
    return true;
}

bool cat_implementation(invocation_t *invocation, error_message_t **error_message) {
    cat_context_t context;
    file_reference_command_t command = {cat_setup, cat_execute, NULL, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(cat, "show contents of file in archive");
