#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

bool extra_field_set_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref;
    extra_field_options_t *options;
    zip_uint16_t length;
    const zip_uint8_t *data;

    if ((file_ref = file_reference_create(invocation, error_message)) == NULL) {
        return false;
    }
    if ((options = extra_field_options_create(invocation, error_message)) == NULL) {
        file_reference_free(file_ref);
        return false;
    }

    if ((data = decode_hex(invocation->arguments[2].string, error_message)) == NULL) {
        result = false;
    }
    else {
        length = strlen((const char *)data) / 2;

        if (zip_file_extra_field_set(archive, file_ref->index, invocation->arguments[1].u16, invocation->arguments[3].u16, data, length, options->flags) < 0) {
            *error_message = error_message_new_format("can't set extra field for %s: %s", file_ref->error_info, zip_strerror(archive));
            result = false;
        }
    }

    file_reference_free(file_ref);
    extra_field_options_free(options);

    return result;
}

/* clang-format off */
argument_definition_t extra_field_set_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"id", "ID of the extra field", argument_type_uint16, true, {.u16 = 0}},
    {"data", "data for the extra field (in hex format)", argument_type_string, true, {.string = NULL}},
    {"index", "index of the extra field to replace (default: create new extra field)", argument_type_uint16, false, {.u16 = ZIP_EXTRA_FIELD_NEW}},
};

option_definition_t extra_field_set_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_OPTIONS,
};
/* clang-format on */

COMMAND(extra_field_set, "set extra field for file in archive");
