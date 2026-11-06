#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

#define ARGUMENT_METHOD 1
#define ARGUMENT_PASSWORD 2

/* clang-format off */
argument_definition_t file_encryption_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"method", "encryption method for the file in the archive", argument_type_encryption_method, ARGUMENT_OPTIONAL, {.u64 = ZIP_UINT64_MAX}},
    {"password", "password for the file in the archive", argument_type_string, ARGUMENT_OPTIONAL, {.string = NULL}}
};

option_definition_t file_encryption_options[] = {
    FILE_REFERENCE_OPTIONS,
};
/* clang-format on */

bool file_encryption_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (invocation->arguments[ARGUMENT_METHOD].u64 == ZIP_UINT64_MAX) {
        zip_stat_t stat;
        argument_t encryption_method;
        if (zip_stat_index(archive, file_ref->index, get_flags(invocation), &stat) < 0) {
            *error_message = error_message_new_format("can't get file info for %s: %s", file_ref->error_info, zip_strerror(archive));
            return false;
        }

        encryption_method.u16 = stat.encryption_method;
        printf("%s\n", enum_argument_to_string(argument_type_encryption_method, encryption_method, true));
        return true;
    }
    else {
        if (zip_file_set_encryption(archive, file_ref->index, invocation->arguments[ARGUMENT_METHOD].u16, invocation->arguments[ARGUMENT_PASSWORD].string) < 0) {
            *error_message = error_message_new_format("can't set encryption method for %s: %s", file_ref->error_info, zip_strerror(archive));
            result = false;
        }
    }

    file_reference_free(file_ref);

    return result;
}

COMMAND(file_encryption, "get or set encryption method for file in archive");
