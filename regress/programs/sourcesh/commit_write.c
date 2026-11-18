#include "sourcesh.h"

/* clang-format off */
argument_definition_t commit_write_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool commit_write_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    if (zip_source_commit_write(source) < 0) {
        *error_message = error_message_new_format("can't commit written data to source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(commit_write, "Commit writing to a source");
