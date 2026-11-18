#include "sourcesh.h"

#include <stdlib.h>
#include <string.h>

/* clang-format off */

#define ARGUMENT_LOWER_SOURCE 1
#define ARGUMENT_START 2
#define ARGUMENT_LENGTH 3

const argument_definition_t window_source_arguments[] = {
    CREATE_SOURCE_ARGUMENTS,
    { "source", "name of lower source", argument_type_source, ARGUMENT_REQUIRED, {.pointer = NULL} },
    { "start", "start offset", argument_type_uint64, ARGUMENT_REQUIRED, {.u64 = 0} },
    { "length", "length of the window", argument_type_uint64, ARGUMENT_OPTIONAL, {.i64 = -1} }
};
/* clang-format on */

bool window_source_implementation(invocation_t *invocation, error_message_t **error_message) {
    const char *source_name = invocation->arguments[ARGUMENT_SOURCE].string;
    zip_source_t *lower_source = invocation->arguments[ARGUMENT_LOWER_SOURCE].pointer;
    zip_uint64_t start = invocation->arguments[ARGUMENT_START].u64;
    zip_int64_t length = invocation->arguments[ARGUMENT_LENGTH].i64;
    zip_error_t zip_error;
    zip_source_t *source;

    if (is_source_opened(source_name)) {
        *error_message = error_message_new_format("source '%s' already created", source_name);
        return false;
    }

    zip_error_init(&zip_error);
    source = zip_source_window_create(lower_source, start, length, &zip_error);
    if (!source) {
        *error_message = error_message_new_format("failed to create window source: %s", zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return false;
    }
    zip_error_fini(&zip_error);

    if (!add_opened_source(source_name, source, error_message)) {
        zip_source_free(source);
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(window_source, "Create a window source");
