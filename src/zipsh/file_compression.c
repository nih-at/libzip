#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

bool file_compression_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (invocation->arguments[1].u64 == ZIP_UINT64_MAX) {
        zip_stat_t stat;
        if (zip_stat_index(archive, file_ref->index, get_flags(invocation), &stat) < 0) {
            *error_message = error_message_new_format("can't get file info for %s: %s", file_ref->error_info, zip_strerror(archive));
            return false;
        }

        argument_t compression_method;
        compression_method.u16 = stat.comp_method;
        printf("%s\n", enum_argument_to_string(argument_type_compression_method, compression_method, true));
        return true;
    }
    else {
        if (zip_set_file_compression(archive, file_ref->index, invocation->arguments[1].u16, invocation->arguments[2].u32) < 0) {
            *error_message = error_message_new_format("can't set compression method for %s: %s", file_ref->error_info, zip_strerror(archive));
            result = false;
        }
    }

    file_reference_free(file_ref);

    return result;
}

/* clang-format off */
argument_definition_t file_compression_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"method", "compression method for the file in the archive", argument_type_compression_method, false, {.u64 = ZIP_UINT64_MAX}},
    {"compression_flags", "compression flags for the file in the archive", argument_type_uint32, false, {.u32 = 0}}
};

option_definition_t file_compression_options[] = {
    FILE_REFERENCE_OPTIONS,
};
/* clang-format on */

COMMAND(file_compression, "get or set compression method for file in archive");
