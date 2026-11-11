#include <errno.h>
#include <string.h>

#include "zipsh.h"

/* clang-format off */
argument_definition_t delete_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t delete_options[] = {
    FILE_REFERENCE_OPTIONS
};
/* clang-format on */


bool delete_execute(invocation_t *invocation, void *context, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    if (zip_delete(archive, file_ref->index) < 0) {
        *error_message = error_message_new_format("can't delete file %s: %s", file_ref->error_info, zip_strerror(archive));
        return false;
    }

    return true;
}

bool delete_implementation(invocation_t *invocation, error_message_t **error_message) {
    file_reference_command_t delete_command = {NULL, delete_execute, NULL, NULL};
    return file_reference_command_execute(&delete_command, invocation, error_message);
}

COMMAND(delete, "delete file from archive");
