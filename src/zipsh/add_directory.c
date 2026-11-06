#include <errno.h>
#include <string.h>

#include "zipsh.h"

#define ARGUMENT_NAME 0

/* clang-format off */
argument_definition_t add_directory_arguments[] = {
    {"name", "directory to add", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
};

option_definition_t add_directory_options[] = {ENCODING_FLAGS_OPTIONS};
/* clang-format on */

bool add_directory_implementation(invocation_t *invocation, error_message_t **error_message) {
    if (zip_dir_add(archive, invocation->arguments[ARGUMENT_NAME].string, get_flags(invocation)) < 0) {
        *error_message = error_message_new_format("can't add directory %s: %s", invocation->arguments[ARGUMENT_NAME].string, zip_strerror(archive));
        return false;
    }

    return true;
}

COMMAND(add_directory, "add directory to archive");
