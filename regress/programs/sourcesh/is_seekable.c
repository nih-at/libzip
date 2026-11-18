#include "sourcesh.h"

#include <stdio.h>
#include <stdlib.h>

/* clang-format off */
argument_definition_t is_seekable_arguments[] = {
    SOURCE_ARGUMENTS,
};
/* clang-format on */

bool is_seekable_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    int ret;


    if ((ret = zip_source_is_seekable(source)) < 0) {
        *error_message = error_message_new_format("can't check if source is seekable: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    printf("%s\n", ret ? "true" : "false");

    return true;
}

COMMAND_NO_OPTIONS(is_seekable, "Check if source is seekable");
