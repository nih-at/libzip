#include <errno.h>
#include <string.h>

#include "zipsh.h"

bool add_directory_implementation(invocation_t *invocation, error_message_t **error_message) {
    if (zip_dir_add(archive, invocation->arguments[0].string, get_flags(invocation)) < 0) {
        *error_message = error_message_new_format("can't add directory %s: %s", invocation->arguments[0].string, zip_strerror(archive));
        return false;
    }

    return true;
}

/* clang-format off */
argument_definition_t add_directory_arguments[] = {
    {"name", "directory to add", argument_type_string, true, {.string = NULL}},
};

option_definition_t add_directory_options[] = {ENCODING_FLAGS_OPTIONS};
/* clang-format on */

COMMAND(add_directory, "add directory to archive");
