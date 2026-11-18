#include "sourcesh.h"

/* clang-format off */

#define ARGUMENT_FILENAME 1

const argument_definition_t file_source_arguments[] = {
    CREATE_SOURCE_ARGUMENTS,
    { "filename", "file to use", argument_type_filename }
};
/* clang-format on */

bool file_source_implementation(invocation_t *invocation, error_message_t **error_message) {
    const char *source_name = invocation->arguments[ARGUMENT_SOURCE].string;
    const char *filename = invocation->arguments[ARGUMENT_FILENAME].string;
    zip_error_t zip_error;
    zip_source_t *source;

    if (is_source_opened(source_name)) {
        *error_message = error_message_new_format("source '%s' already created", source_name);
        return false;
    }

    zip_error_init(&zip_error);
    source = zip_source_file_create(filename, 0, 0, &zip_error);
    if (!source) {
        *error_message = error_message_new_format("failed to create source from file '%s': %s", filename, zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return false;
    }
    zip_error_fini(&zip_error);

    if (!add_opened_source(source_name, source, error_message)) {
        zip_source_free(source);
        return false;
    }

    return true;
}

COMMAND_NO_OPTIONS(file_source, "Create a file source");
