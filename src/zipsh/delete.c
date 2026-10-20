#include <errno.h>
#include <string.h>

#include "zipsh.h"

bool delete_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (zip_delete(archive, file_ref->index) < 0) {
        *error_message = error_message_new_format("can't delete %s: %s", file_ref->error_info, zip_strerror(archive));
        result = false;
    }

    file_reference_free(file_ref);

    return result;
}

/* clang-format off */
argument_definition_t delete_arguments[] = {
    {"file", "file to delete", argument_type_filename, true, {.string = NULL}},
};

option_definition_t delete_options[] = {
    {'i', "index", NULL, "file argument is index instead of name", NULL},
};
/* clang-format on */

COMMAND(delete, "delete file from archive");
