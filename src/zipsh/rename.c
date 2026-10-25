#include <errno.h>
#include <string.h>

#include "zipsh.h"

bool rename_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (zip_file_rename(archive, file_ref->index, invocation->arguments[1].string, 0) < 0) {
        *error_message = error_message_new_format("can't rename %s to '%s': %s", file_ref->error_info, invocation->arguments[1].string, zip_strerror(archive));
        result = false;
    }

    file_reference_free(file_ref);

    return result;
}

/* clang-format off */
argument_definition_t rename_arguments[] = {
    {"file", "file to rename", argument_type_filename, true, {.string = NULL}},
    {"new_name", "new name for the file in the archive", argument_type_string, false, {.string = NULL}},
};

option_definition_t rename_options[] = {
    {'i', "index", NULL, "file argument is index instead of name", NULL},
};
/* clang-format on */

COMMAND(rename, "rename file in archive");
