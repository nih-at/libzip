#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

/* clang-format off */
argument_definition_t extra_field_count_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t extra_field_count_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_ID_OPTIONS,
};

struct context {
    extra_field_options_t *options;
};

typedef struct context context_t;
/* clang-format on */

bool extra_field_count_setup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    if ((context->options = extra_field_options_create(invocation, error_message)) == NULL) {
        return false;
    }

    return true;
}

bool extra_field_count_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;
    zip_int16_t count;

    if (context->options->id_specified) {
        count = zip_file_extra_fields_count_by_id(archive, file_ref->index, context->options->id, context->options->flags);
    }
    else {
        count = zip_file_extra_fields_count(archive, file_ref->index, context->options->flags);
    }
    if (count < 0) {
        *error_message = error_message_new_format("can't get extra field count for %s: %s", file_ref->error_info, zip_strerror(archive));
        return false;
    }
    else {
        printf("%d\n", count);
    }

    return true;
}

bool extra_field_count_cleanup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    extra_field_options_free(context->options);

    return true;
}

bool extra_field_count_implementation(invocation_t *invocation, error_message_t **error_message) {
    context_t context;
    file_reference_command_t command = {extra_field_count_setup, extra_field_count_execute, extra_field_count_cleanup, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(extra_field_count, "get count of extra fields for file in archive");
