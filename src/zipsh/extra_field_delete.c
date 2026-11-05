#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

bool extra_field_delete_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref;
    extra_field_options_t *options;
    int ret;

    if ((file_ref = file_reference_create(invocation, error_message)) == NULL) {
        return false;
    }
    if ((options = extra_field_options_create(invocation, error_message)) == NULL) {
        file_reference_free(file_ref);
        return false;
    }

    if (options->id_specified) {
        ret = zip_file_extra_field_delete_by_id(archive, file_ref->index, options->id, invocation->arguments[1].u16, options->flags);
    }
    else {
        ret = zip_file_extra_field_delete(archive, file_ref->index, invocation->arguments[1].u16, options->flags);
    }
    if (ret < 0) {
        *error_message = error_message_new_format("can't delete extra field for %s: %s", file_ref->error_info, zip_strerror(archive));
        result = false;
    }

    file_reference_free(file_ref);
    extra_field_options_free(options);

    return result;
}

/* clang-format off */
argument_definition_t extra_field_delete_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"index", "index of the extra field", argument_type_uint16, true, {.u16 = 0}}
};

option_definition_t extra_field_delete_options[] = {
    FILE_REFERENCE_OPTIONS,
    EXTRA_FIELD_ID_OPTIONS,
};
/* clang-format on */

COMMAND(extra_field_delete, "delete extra field for file in archive");
