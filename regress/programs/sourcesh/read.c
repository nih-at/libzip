#include "sourcesh.h"

#include <stdio.h>
#include <stdlib.h>

/* clang-format off */
#define ARGUMENT_LENGTH 1

argument_definition_t read_arguments[] = {
    SOURCE_ARGUMENTS,
    {"length", "length to read", argument_type_uint64, ARGUMENT_REQUIRED, {.u64 = 0}}
};
/* clang-format on */

bool read_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    zip_uint64_t length = invocation->arguments[ARGUMENT_LENGTH].u64;
    zip_int64_t read_length;
    uint8_t *data = malloc(length);

    if (data == NULL) {
        *error_message = error_message_new("out of buffer");
        return false;
    }

    if ((read_length = zip_source_read(source, data, length)) < 0) {
        *error_message = error_message_new_format("can't read from source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    print_hex(data, read_length);

    free(data);
    return true;
}

COMMAND_NO_OPTIONS(read, "Read from a source");
