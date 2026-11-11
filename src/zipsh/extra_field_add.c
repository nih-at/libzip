#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "zip.h"
#include "zipsh.h"

#define ARGUMENT_ID 1
#define ARGUMENT_DATA 2

/* clang-format off */
argument_definition_t extra_field_add_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"id", "ID of the extra field", argument_type_uint16, ARGUMENT_REQUIRED, {.u16 = 0}},
    {"data", "data for the extra field (in hex format)", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
};

option_definition_t extra_field_add_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_OPTIONS,
};

struct context {
    extra_field_options_t *options;
    zip_uint16_t length;
    const zip_uint8_t *data;
};

typedef struct context context_t;
/* clang-format on */


bool extra_field_add_setup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    if ((context->options = extra_field_options_create(invocation, error_message)) == NULL) {
        return false;
    }

    if ((context->data = decode_hex(invocation->arguments[ARGUMENT_DATA].string, error_message)) == NULL) {
        return false;
    }
    else {
        size_t data_length = strlen(invocation->arguments[ARGUMENT_DATA].string) / 2;
        if (data_length > ZIP_UINT16_MAX) {
            *error_message = error_message_new_format("data too long");
            return false;
        }
        context->length = (zip_uint16_t)data_length;
    }

    return true;
}

bool extra_field_add_cleanup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    extra_field_options_free(context->options);
    if (context->data != NULL) {
        free((void *)context->data);
    }

    return true;
}

bool extra_field_add_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    if (zip_file_extra_field_set(archive, file_ref->index, invocation->arguments[ARGUMENT_ID].u16, ZIP_EXTRA_FIELD_NEW, context->data, context->length, context->options->flags) < 0) {
        *error_message = error_message_new_format("can't add extra field for %s: %s", file_ref->error_info, zip_strerror(archive));
        return false;
    }

    return true;
}

bool extra_field_add_implementation(invocation_t *invocation, error_message_t **error_message) {
    context_t context = {NULL, 0, NULL};
    file_reference_command_t command = {extra_field_add_setup, extra_field_add_execute, extra_field_add_cleanup, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(extra_field_add, "add extra field for file in archive");
