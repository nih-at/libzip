#include "sourcesh.h"

/* clang-format off */
argument_definition_t close_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool close_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    if (zip_source_close(source) < 0) {
        *error_message = error_message_new_format("can't close source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(close, "Close a source for reading");
