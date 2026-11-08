#include "zipsh_regress.h"


#define ARGUMENT_NAME 0
#define ARGUMENT_LENGTH 1

/* clang-format off */
argument_definition_t add_nul_arguments[] = {
    {"name", "name of new file in archive", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
    {"length", "number of bytes to read from the file", argument_type_int64, ARGUMENT_OPTIONAL, {.i64 = -1}},
};

option_definition_t add_nul_options[] = {
    ENCODING_FLAGS_OPTIONS,
};
/* clang-format on */

bool add_nul_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source;
    zip_int64_t len = invocation->arguments[ARGUMENT_LENGTH].i64;
    source = source_nul(archive, len);
    if (source == NULL) {
        *error_message = error_message_new_format("can't create NUL source: %s", zip_strerror(archive));
        return false;
    }

    if (zip_file_add(archive, invocation->arguments[ARGUMENT_NAME].string, source, 0) < 0) {
        *error_message = error_message_new_format("can't add NUL source to archive: %s", zip_strerror(archive));
        zip_source_free(source);
        return false;
    }

    return true;
}

COMMAND(add_nul, "add file consisting of NUL bytes to archive");
