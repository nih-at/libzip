#include "zipsh.h"

#include <string.h>

#include "zip.h"


/* clang-format off */
argument_definition_t lookup_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t lookup_options[] = {
    FLAGS_OPTIONS,
};

typedef struct context context_t;
/* clang-format on */

bool lookup_execute(invocation_t *invocation, void *context, file_matches_t *matches, file_reference_t *file_ref, error_message_t *error_message) {
    (void)context;

    printf("%" PRIu64 ": %s\n", file_ref->index, file_ref->name);

    return true;
}

bool lookup_implementation(invocation_t *invocation, error_message_t *error_message) {
    file_reference_command_t command = {NULL, lookup_execute, NULL, NULL};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(lookup, "lookup file by name");
