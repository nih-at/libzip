#include "sourcesh.h"
#include "zipconf.h"

#include <stdlib.h>
#include <string.h>

/* clang-format off */
#define ARGUMENT_DATA 1

argument_definition_t write_arguments[] = {
    SOURCE_ARGUMENTS,
    {"data", "data to write in hex format", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL }}
};

/* clang-format on */

bool write_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    const char *hex_data = invocation->arguments[ARGUMENT_DATA].string;
    zip_uint8_t *data = decode_hex(hex_data, error_message);
    zip_int64_t written;

    if (data == NULL) {
        return false;
    }

    written = zip_source_write(source, data, strlen(hex_data) / 2);
    free(data);
    if (written < 0) {
        *error_message = error_message_new_format("can't write to source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }
    else if (written != (zip_int64_t)(strlen(hex_data) / 2)) {
        *error_message = error_message_new_format("can't write all data to source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(write, "Begin writing to a source");
