#include "zipsh.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

#define ARGUMENT_NAME 0
#define ARGUMENT_DATA 1

/* clang-format off */
argument_definition_t add_data_arguments[] = {
    {"name", "name of new file in archive", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
    {"data", "file or data to add", argument_type_filename, ARGUMENT_REQUIRED, {.string = NULL}},
};

option_definition_t add_data_options[] = {
    ENCODING_FLAGS_OPTIONS,
    {'h', "hex", NULL, "data is in hex format"}
};
/* clang-format on */

bool add_data_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source;
    const char *input = invocation->arguments[ARGUMENT_DATA].string;
    zip_uint8_t *data = NULL;
    size_t length = strlen(input);
    bool is_hex = invocation_get_option(invocation, 'h');

    if (is_hex) {
        if ((data = decode_hex(input, error_message)) == NULL) {
            return false;
        }
        length /= 2;
    }
    else {
        if ((data = (zip_uint8_t *)malloc(length)) == NULL) {
            *error_message = error_message_new_format("can't allocate memory");
            return false;
        }
        memcpy(data, input, length);
    }

    source = zip_source_buffer(archive, data, length, 1);

    if (source == NULL) {
        if (error_message) {
            *error_message = error_message_new_format("can't create zip source: '%s'", zip_strerror(archive));
        }
        free(data);
        return false;
    }
    if (zip_file_add(archive, invocation->arguments[ARGUMENT_NAME].string, source, get_flags(invocation)) < 0) {
        if (error_message) {
            *error_message = error_message_new_format("can't add file to archive: '%s'", zip_strerror(archive));
        }
        zip_source_free(source);
        return false;
    }

    return true;
}

COMMAND(add_data, "add file to archive from provided data");
