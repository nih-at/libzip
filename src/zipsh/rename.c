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

bool rename_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (zip_file_rename(archive, file_ref->index, invocation->arguments[ARGUMENT_NEW_NAME].string, 0) < 0) {
        *error_message = error_message_new_format("can't rename %s to '%s': %s", file_ref->error_info, invocation->arguments[ARGUMENT_NEW_NAME].string, zip_strerror(archive));
        result = false;
    }

    file_reference_free(file_ref);

    return result;
}

COMMAND(rename, "rename file in archive");
