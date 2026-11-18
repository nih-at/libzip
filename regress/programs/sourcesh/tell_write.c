#include "sourcesh.h"

#include <stdio.h>

/* clang-format off */
argument_definition_t tell_write_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool tell_write_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    zip_int64_t offset = zip_source_tell_write(source);
    if (offset < 0) {
        *error_message = error_message_new_format("can't tell position: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    printf("%" PRId64 "\n", offset);
    return true;
}

COMMAND_NO_OPTIONS(tell_write, "Tell the current position within a source");
