#include "sourcesh.h"

/* clang-format off */
#define ARGUMENT_OFFSET 1
#define ARGUMENT_WHENCE 2

argument_definition_t seek_write_arguments[] = {
    SOURCE_ARGUMENTS,
    {"offset", "offset to seek to", argument_type_int64,ARGUMENT_REQUIRED, {.u64 = 0} },
    {"whence", "seek whence", argument_type_seek_whence, ARGUMENT_OPTIONAL, { .i16 = SEEK_SET } }
};
/* clang-format on */

bool seek_write_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;

    if (zip_source_seek_write(source, invocation->arguments[ARGUMENT_OFFSET].i64, invocation->arguments[ARGUMENT_WHENCE].i16) < 0) {
        *error_message = error_message_new_format("can't seek: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(seek_write, "Seek within a source");
