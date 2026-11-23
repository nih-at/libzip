#include "zipsh.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

#define ARGUMENT_DATA 1

/* clang-format off */
argument_definition_t replace_data_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"data", "file or data to add", argument_type_filename, ARGUMENT_REQUIRED, {.string = NULL}},
};

option_definition_t replace_data_options[] = {
    FILE_REFERENCE_OPTIONS,
    {'h', "hex", NULL, "data is in hex format"}
};

struct context {
    const zip_uint8_t *data;
    size_t length;
    zip_uint8_t *allocated_data;
};

typedef struct context context_t;
/* clang-format on */

bool replace_data_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t *error_message) {
    context_t *context = (context_t *)context_v;
    zip_source_t *source;
    zip_uint8_t *data;

    if ((data = (zip_uint8_t *)malloc(context->length)) == NULL) {
        error_message_set_format(error_message, "can't allocate memory");
        return false;
    }
    memcpy(data, context->data, context->length);

    if ((source = zip_source_buffer(archive, data, context->length, 1)) == NULL) {
        error_message_set_format(error_message, "can't create zip source: '%s'", zip_strerror(archive));
        free(data);
        return false;
    }
    if (zip_file_replace(archive, file_ref->index, source, matches->flags) < 0) {
        error_message_set_format(error_message, "can't add file to archive: '%s'", zip_strerror(archive));
        zip_source_free(source);
        return false;
    }

    return true;
}


bool replace_data_setup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t *error_message) {
    context_t *context = (context_t *)context_v;
    const char *input = invocation->arguments[ARGUMENT_DATA].string;
    bool is_hex = invocation_get_option(invocation, 'h');

    context->data = (const zip_uint8_t *)input;
    context->length = strlen(input);

    if (is_hex) {
        if ((context->allocated_data = decode_hex(input, error_message)) == NULL) {
            return false;
        }
        context->data = context->allocated_data;
        context->length /= 2;
    }
    return true;
}

bool replace_data_cleanup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t *error_message) {
    context_t *context = (context_t *)context_v;

    free(context->allocated_data);
    return true;
}


bool replace_data_implementation(invocation_t *invocation, error_message_t *error_message) {
    context_t context = {NULL, 0, NULL};
    file_reference_command_t command = {replace_data_setup, replace_data_execute, replace_data_cleanup, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(replace_data, "replace file data");
