#include <errno.h>
#include <string.h>

#include "zipsh.h"

#define ARGUMENT_NEW_NAME 1

/* clang-format off */
argument_definition_t rename_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"new_name", "new name for the file in the archive", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
};

option_definition_t rename_options[] = {
    FILE_REFERENCE_OPTIONS
};
/* clang-format on */

bool rename_execute(invocation_t *invocation, void *context, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    (void)context;
    (void)matches;

    if (zip_file_rename(archive, file_ref->index, invocation->arguments[ARGUMENT_NEW_NAME].string, 0) < 0) {
        *error_message = error_message_new_format("can't rename file %s to '%s': %s", file_ref->error_info, invocation->arguments[ARGUMENT_NEW_NAME].string, zip_strerror(archive));
        return false;
    }

    return true;
}

bool rename_implementation(invocation_t *invocation, error_message_t **error_message) {
    file_reference_command_t command = {NULL, rename_execute, NULL, NULL};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(rename, "rename file in archive");
