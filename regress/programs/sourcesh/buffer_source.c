#include "sourcesh.h"

#include <stdlib.h>
#include <string.h>

/* clang-format off */

#define ARGUMENT_DATA 1

const argument_definition_t buffer_source_arguments[] = {
    CREATE_SOURCE_ARGUMENTS,
    { "data", "data to use in hex format", argument_type_string, ARGUMENT_OPTIONAL, {.string= ""} }
};
/* clang-format on */

bool buffer_source_implementation(invocation_t *invocation, error_message_t **error_message) {
    const char *source_name = invocation->arguments[ARGUMENT_SOURCE].string;
    const char *hex_data = invocation->arguments[ARGUMENT_DATA].string;
    zip_uint8_t *data = decode_hex(hex_data, error_message);
    zip_error_t zip_error;
    zip_source_t *source;

    if (data == NULL) {
        return false;
    }

    if (is_source_opened(source_name)) {
        *error_message = error_message_new_format("source '%s' already created", source_name);
        return false;
    }

    zip_error_init(&zip_error);
    source = zip_source_buffer_create(data, strlen(hex_data) / 2, 1, &zip_error);
    if (!source) {
        *error_message = error_message_new_format("failed to create source from buffer: %s", zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        free(data);
        return false;
    }
    zip_error_fini(&zip_error);

    if (!add_opened_source(source_name, source, error_message)) {
        zip_source_free(source);
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(buffer_source, "Create a buffer source");
