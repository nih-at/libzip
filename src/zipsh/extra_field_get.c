#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

#define ARGUMENT_INDEX 1

/* clang-format off */
argument_definition_t extra_field_get_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"index", "index of the extra field", argument_type_uint16, ARGUMENT_REQUIRED, {.u16 = 0}}
};

option_definition_t extra_field_get_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_ID_OPTIONS,
};

struct context {
    extra_field_options_t *options;
};

typedef struct context context_t;
/* clang-format on */

bool extra_field_get_setup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    if ((context->options = extra_field_options_create(invocation, error_message)) == NULL) {
        return false;
    }

    return true;
}

bool extra_field_get_cleanup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    extra_field_options_free(context->options);

    return true;
}

bool extra_field_get_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;
    zip_uint16_t length;
    const zip_uint8_t *data;

    if (context->options->id_specified) {
        data = zip_file_extra_field_get_by_id(archive, file_ref->index, context->options->id, invocation->arguments[ARGUMENT_INDEX].u16, &length, context->options->flags);
    }
    else {
        data = zip_file_extra_field_get(archive, file_ref->index, invocation->arguments[ARGUMENT_INDEX].u16, &context->options->id, &length, context->options->flags);
    }
    if (data == NULL) {
        *error_message = error_message_new_format("can't get extra field for %s: %s", file_ref->error_info, zip_strerror(archive));
        return false;
    }
    else {
        if (!context->options->id_specified) {
            printf("0x%04x: ", context->options->id);
        }
        print_hex(data, length);
    }

    return true;
}

bool extra_field_get_implementation(invocation_t *invocation, error_message_t **error_message) {
    context_t context;
    file_reference_command_t command = {extra_field_get_setup, extra_field_get_execute, extra_field_get_cleanup, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(extra_field_get, "get extra field for file in archive");
