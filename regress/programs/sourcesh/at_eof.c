#include "sourcesh.h"

#include <stdio.h>
#include <stdlib.h>

/* clang-format off */
argument_definition_t at_eof_arguments[] = {
    SOURCE_ARGUMENTS,
};
/* clang-format on */

bool at_eof_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    int ret;


    if ((ret = zip_source_at_eof(source)) < 0) {
        *error_message = error_message_new_format("can't check EOF in source: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    printf("%s\n", ret ? "true" : "false");

    return true;
}

COMMAND_NO_OPTIONS(at_eof, "Check if at end of source");
