#include "sourcesh.h"

/* clang-format off */
argument_definition_t rollback_write_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool rollback_write_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    zip_source_rollback_write(source);

    return true;
}

COMMAND_NO_OPTIONS(rollback_write, "Rollback writing to a source");
