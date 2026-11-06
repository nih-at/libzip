#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

/* clang-format off */
argument_definition_t extra_field_count_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t extra_field_count_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_ID_OPTIONS,
};
/* clang-format on */

bool extra_field_count_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref;
    extra_field_options_t *options;
    zip_int16_t count;

    if ((file_ref = file_reference_create(invocation, error_message)) == NULL) {
        return false;
    }
    if ((options = extra_field_options_create(invocation, error_message)) == NULL) {
        file_reference_free(file_ref);
        return false;
    }

    if (options->id_specified) {
        count = zip_file_extra_fields_count_by_id(archive, file_ref->index, options->id, options->flags);
    }
    else {
        count = zip_file_extra_fields_count(archive, file_ref->index, options->flags);
    }
    if (count < 0) {
        *error_message = error_message_new_format("can't get extra field count for %s: %s", file_ref->error_info, zip_strerror(archive));
        result = false;
    }
    else {
        printf("%d\n", count);
    }

    file_reference_free(file_ref);
    extra_field_options_free(options);

    return result;
}

COMMAND(extra_field_count, "get count of extra fields for file in archive");
