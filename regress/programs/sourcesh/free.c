#include "sourcesh.h"

/* clang-format off */
argument_definition_t free_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool free_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    return free_opened_source(source, error_message);
}


COMMAND_NO_OPTIONS(free, "Free a source");
